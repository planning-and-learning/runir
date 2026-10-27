#include "kr/parser/datalog.hpp"

#include "kr/parser/ext_constructors.hpp"
#include "kr/parser/resolution.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/errors.hpp"

#include <algorithm>
#include <boost/variant/apply_visitor.hpp>
#include <charconv>
#include <concepts>
#include <set>
#include <stdexcept>
#include <string_view>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/database/datalog/construction.hpp>
#include <yggdrasil/database/datalog/evaluator.hpp>

namespace runir::kr::parser::datalog
{

namespace dl = runir::kr::dl;
namespace ydl = ygg::database::datalog;

void FeatureResolver::declare(const ast::Identifier& name, std::function<FeatureReference()> parse)
{
    if (!m_entries.emplace(name.text, Entry { std::move(parse), std::nullopt, false }).second)
        m_diagnostics->throw_at(name, DuplicateDefinitionError("feature", name.text));
}

std::optional<FeatureReference> FeatureResolver::resolve(const ast::Identifier& name)
{
    const auto it = m_entries.find(name.text);
    if (it == m_entries.end())
        return std::nullopt;
    auto& entry = it->second;
    if (entry.value)
        return entry.value;
    if (entry.in_progress)
    {
        auto cycle = std::string();
        for (auto i = std::ranges::find(m_stack, name.text); i != m_stack.end(); ++i)
            cycle += *i + " -> ";
        m_diagnostics->throw_at(name, InvalidExpressionError("Feature `" + name.text + "` depends on itself: " + cycle + name.text + "."));
    }
    entry.in_progress = true;
    m_stack.push_back(name.text);
    auto value = entry.parse();
    m_stack.pop_back();
    entry.in_progress = false;
    entry.value = value;
    return value;
}

namespace
{

namespace xast = runir::kr::ps::ext::dl::ast;
namespace constructors = runir::kr::parser::constructors;

class ProgramBuilder
{
public:
    ProgramBuilder(tyr::formalism::planning::DomainView domain,
                   dl::ConstructorRepository& repository,
                   dl::Builder& builder,
                   FeatureResolver& features,
                   const ext_constructors::ModuleReferences& references,
                   const std::map<std::string, size_t>& arities,
                   const std::set<std::string>& weighted,
                   const DiagnosticContext& diagnostics) :
        m_domain(domain),
        m_context(repository, builder, diagnostics, &references),
        m_features(&features)
    {
        for (const auto& [name, arity] : arities)
        {
            auto data = ydl::checkout<ydl::Relation>(m_datalog_builder);
            data->id = m_derived.size();
            data->arity = arity;
            data->name = name;
            data->weighted = weighted.contains(name);
            m_derived.emplace(name, ydl::insert(datalog(), *data).first.get_index());
        }
    }

    void add_rule(const xast::DatalogRule& syntax)
    {
        m_variables.clear();
        auto rule = ydl::checkout<ydl::Rule>(m_datalog_builder);
        auto head_terms = syntax.head.terms;
        if (syntax.head.min)
            head_terms.push_back(*syntax.head.min);
        rule->head = atom(syntax.head.relation, head_terms, head_relation(syntax.head.relation)).get_index();
        for (const auto& literal : syntax.body)
            boost::apply_visitor(
                [&]<typename T>(const T& node)
                {
                    if constexpr (std::same_as<T, xast::DatalogAggregate>)
                        aggregate(node, rule->aggregates);
                    else
                        condition(node, rule->atoms, rule->negated, rule->comparisons, rule->arithmetic);
                },
                literal);
        try
        {
            m_rules.push_back(ydl::insert(datalog(), *rule).first.get_index());
        }
        catch (const std::invalid_argument& error)
        {
            m_context.diagnostics().throw_at(syntax, InvalidExpressionError(std::string("Datalog rule: ") + error.what()));
        }
    }

    ygg::Index<dl::DatalogProgram> finish(const ast::Identifier& reference, const std::set<std::string>& read_heads)
    {
        auto program = ydl::checkout<ydl::Program>(m_datalog_builder);
        for (const auto rule : m_rules)
            program->rules.push_back(rule);
        const auto view = ydl::insert(datalog(), *program).first;
        try
        {
            // Validates the stratification.
            auto heads = std::vector<ydl::RelationView>();
            for (const auto& [name, relation] : m_derived)
                heads.emplace_back(relation, datalog());
            [[maybe_unused]] const auto evaluator = ydl::Evaluator(view, heads);
        }
        catch (const std::invalid_argument& error)
        {
            m_context.diagnostics().throw_at(reference, InvalidExpressionError(std::string("Datalog program: ") + error.what()));
        }
        auto data = constructors::checkout<dl::DatalogProgram>(m_context);
        data->program = view.get_index();
        for (const auto input : m_inputs)
            data->inputs.push_back(input);
        for (const auto& [name, relation] : m_derived)
            if (read_heads.contains(name))
                data->outputs.push_back(relation);
        return constructors::intern(m_context, *data).get_index();
    }

    ygg::Index<ydl::Relation> derived(const std::string& name) const { return m_derived.at(name); }

private:
    tyr::formalism::planning::DomainView m_domain;
    ext_constructors::ConstructorContext m_context;
    FeatureResolver* m_features;
    ydl::DatalogBuilder m_datalog_builder;
    std::map<std::string, ygg::Index<ydl::Relation>> m_derived;
    std::vector<ygg::Index<dl::Query<>>> m_inputs;
    std::map<ygg::uint_t, ygg::Index<ydl::Relation>> m_input_relations;  ///< By input query index.
    std::map<std::string, ygg::uint_t> m_variables;
    std::vector<ygg::Index<ydl::Rule>> m_rules;

    ydl::DatalogRepository& datalog() { return m_context.repository().get_datalog_repository(); }

    /// A variable, a number (digits), or an object constant.
    ydl::Term term(const ast::Identifier& name)
    {
        if (name.text.starts_with('?'))
            return ydl::Term::variable(m_variables.try_emplace(name.text, ygg::to_uint_t(m_variables.size())).first->second);
        if (std::ranges::all_of(name.text, [](char c) { return c >= '0' && c <= '9'; }))
        {
            const auto text = std::string_view(name.text);
            auto value = ygg::uint_t { 0 };
            const auto [end, error] = std::from_chars(text.begin(), text.end(), value);
            if (error != std::errc() || end != text.end() || value == ydl::Arithmetic::infinity)
                m_context.diagnostics().throw_at(name, InvalidExpressionError("The number " + name.text + " is too large."));
            return ydl::Term::constant(value);
        }
        return ydl::Term::constant(static_cast<ygg::uint_t>(require_object(m_domain, name, m_context.diagnostics())));
    }

    ydl::AtomView atom(const ast::Identifier& name, const std::vector<ast::Identifier>& terms, ygg::Index<ydl::Relation> relation)
    {
        auto data = ydl::checkout<ydl::Atom>(m_datalog_builder);
        data->relation = relation;
        for (const auto& t : terms)
            data->terms.push_back(term(t));
        try
        {
            return ydl::insert(datalog(), *data).first;
        }
        catch (const std::invalid_argument&)
        {
            m_context.diagnostics().throw_at(name, ArityMismatchError(name.text, ydl::RelationView(relation, datalog()).get_arity(), terms.size()));
        }
    }

    ygg::Index<ydl::Relation> head_relation(const ast::Identifier& name)
    {
        const auto it = m_derived.find(name.text);
        if (it == m_derived.end())
            m_context.diagnostics().throw_at(name, InvalidExpressionError("Not a Datalog head: " + name.text));
        return it->second;
    }

    /// The columns _0, ..., _{arity - 1} of an input query, read positionally.
    ygg::IndexList<dl::QueryColumn> columns(size_t arity) const
    {
        auto names = std::vector<runir::kr::dl::grammar::ast::Identifier>(arity);
        for (size_t i = 0; i < arity; ++i)
            names[i].text = "_" + std::to_string(i);
        return constructors::parse_columns(names, m_context);
    }
    template<typename Tag>
    ygg::Index<dl::Query<>> query(ygg::Index<dl::Query<Tag>> index) const
    { return constructors::intern_query(m_context, index).get_index(); }

    /// The input relation of an input query, a new input slot for a new query.
    ygg::Index<ydl::Relation> input(ygg::Index<dl::Query<>> query, const std::string& name)
    {
        if (const auto it = m_input_relations.find(query.get_value()); it != m_input_relations.end())
            return it->second;
        auto data = ydl::checkout<ydl::Relation>(m_datalog_builder);
        data->id = m_inputs.size();
        data->arity = ygg::make_view(query, m_context.repository()).get_schema().size();
        data->input_slot = m_inputs.size();
        data->name = name;
        const auto relation = ydl::insert(datalog(), *data).first.get_index();
        m_inputs.push_back(query);
        m_input_relations.emplace(query.get_value(), relation);
        return relation;
    }

    /// A predicate's atoms in the state, or in the goal's positive or negative literals.
    ygg::Index<ydl::Relation> predicate(const ast::Identifier& name, size_t arity, bool goal, bool polarity = true)
    {
        const auto make = [&]<typename T>(T, auto predicate) -> ygg::Index<dl::Query<>>
        {
            if (goal)
            {
                auto data = constructors::checkout<dl::Query<dl::AtomicGoalTag<T>>>(m_context);
                data->predicate = predicate;
                data->polarity = polarity;
                data->columns = columns(arity);
                return query(constructors::intern(m_context, *data).get_index());
            }
            auto data = constructors::checkout<dl::Query<dl::AtomicStateTag<T>>>(m_context);
            data->predicate = predicate;
            data->columns = columns(arity);
            return query(constructors::intern(m_context, *data).get_index());
        };
        const auto index = runir::kr::parser::resolve_predicate(m_domain, name, arity, goal ? "goal atom" : "atom", m_context.diagnostics(), make);
        return input(index, (goal ? (polarity ? "goal " : "goal not ") : "") + name.text);
    }

    /// A concept or role register or argument of the module, as the constructor that reads it.
    std::optional<FeatureReference> module_reference(const ast::Identifier& name) const
    {
        const auto& references = *m_context.references();
        const auto find = [&](const auto& table) -> std::optional<std::decay_t<decltype(table.by_name.begin()->second)>>
        {
            const auto it = table.by_name.find(name.text);
            return it == table.by_name.end() ? std::nullopt : std::optional(it->second);
        };
        // The record of a register or argument constructor.
        const auto reading = [&]<typename Category, typename Record>(auto reference) -> FeatureReference
        {
            auto data = dl::checkout<Record>(m_context.builder());
            data->reference = reference;
            return constructors::intern_constructor<Category>(m_context, ext_constructors::intern(m_context, *data).get_index()).get_index();
        };
        if (const auto r = find(references.concept_registers))
            return reading.template operator()<dl::ConceptTag, dl::Concept<dl::RegisterTag>>(*r);
        if (const auto r = find(references.role_registers))
            return reading.template operator()<dl::RoleTag, dl::Role<dl::RegisterTag>>(*r);
        if (const auto r = find(references.concept_arguments))
            return reading.template operator()<dl::ConceptTag, dl::Concept<dl::ArgumentTag<dl::ConceptTag>>>(*r);
        if (const auto r = find(references.role_arguments))
            return reading.template operator()<dl::RoleTag, dl::Role<dl::ArgumentTag<dl::RoleTag>>>(*r);
        if (const auto r = find(references.boolean_arguments))
            return reading.template operator()<dl::BooleanTag, dl::Boolean<dl::ArgumentTag<dl::BooleanTag>>>(*r);
        if (const auto r = find(references.numerical_arguments))
            return reading.template operator()<dl::NumericalTag, dl::Numerical<dl::ArgumentTag<dl::NumericalTag>>>(*r);
        return std::nullopt;
    }

    /// The relation of a body atom: a head, a feature, a register or argument of the module, or a domain predicate.
    ygg::Index<ydl::Relation> body_relation(const ast::Identifier& name, size_t arity)
    {
        if (const auto it = m_derived.find(name.text); it != m_derived.end())
            return it->second;
        auto feature = m_features->resolve(name);
        if (!feature)
            feature = module_reference(name);
        if (feature)
        {
            const auto index = std::visit(
                [&]<typename T>(ygg::Index<T> resolved) -> ygg::Index<dl::Query<>>
                {
                    if constexpr (std::same_as<T, dl::Constructor<dl::ConceptTag>>)
                    {
                        auto data = constructors::checkout<dl::Query<dl::QueryConceptTag>>(m_context);
                        data->arg = resolved;
                        data->columns = columns(1);
                        return query(constructors::intern(m_context, *data).get_index());
                    }
                    else if constexpr (std::same_as<T, dl::Constructor<dl::RoleTag>>)
                    {
                        auto data = constructors::checkout<dl::Query<dl::QueryRoleTag>>(m_context);
                        data->arg = resolved;
                        data->columns = columns(2);
                        return query(constructors::intern(m_context, *data).get_index());
                    }
                    else if constexpr (std::same_as<T, dl::Constructor<dl::BooleanTag>>)
                    {
                        auto data = constructors::checkout<dl::Query<dl::QueryBooleanTag>>(m_context);
                        data->arg = resolved;
                        return query(constructors::intern(m_context, *data).get_index());
                    }
                    else if constexpr (std::same_as<T, dl::Constructor<dl::NumericalTag>>)
                    {
                        auto data = constructors::checkout<dl::Query<dl::QueryNumericalTag>>(m_context);
                        data->arg = resolved;
                        data->columns = columns(1);
                        return query(constructors::intern(m_context, *data).get_index());
                    }
                    else
                        return resolved;
                },
                *feature);
            return input(index, name.text);
        }
        return predicate(name, arity, false);
    }

    /// Adds a literal other than an aggregate: an atom, a goal atom, a negated atom, a comparison or arithmetic.
    void condition(const xast::DatalogCondition& syntax,
                   ygg::IndexList<ydl::Atom>& atoms,
                   ygg::IndexList<ydl::Atom>& negated,
                   cista::offset::vector<ydl::Comparison>& comparisons,
                   cista::offset::vector<ydl::Arithmetic>& arithmetic)
    {
        boost::apply_visitor(
            [&]<typename T>(const T& node)
            {
                if constexpr (std::same_as<T, xast::DatalogNegation>)
                    negated.push_back(atom(node.atom.relation, node.atom.terms, body_relation(node.atom.relation, node.atom.terms.size())).get_index());
                else if constexpr (std::same_as<T, xast::DatalogGoalAtom>)
                    atoms.push_back(atom(node.predicate, node.terms, predicate(node.predicate, node.terms.size(), true, !node.negative)).get_index());
                else if constexpr (std::same_as<T, xast::DatalogComparison>)
                {
                    using Operator = xast::DatalogComparisonOperator;
                    using Kind = ydl::Comparison::Kind;
                    const auto lhs = term(node.lhs), rhs = term(node.rhs);
                    switch (node.op)
                    {
                        case Operator::EQUAL:
                            comparisons.push_back(ydl::Comparison { Kind::EQUAL, lhs, rhs });
                            break;
                        case Operator::NOT_EQUAL:
                            comparisons.push_back(ydl::Comparison { Kind::NOT_EQUAL, lhs, rhs });
                            break;
                        case Operator::LESS:
                            comparisons.push_back(ydl::Comparison { Kind::LESS, lhs, rhs });
                            break;
                        case Operator::LESS_EQUAL:
                            comparisons.push_back(ydl::Comparison { Kind::LESS_EQUAL, lhs, rhs });
                            break;
                        case Operator::GREATER:
                            comparisons.push_back(ydl::Comparison { Kind::LESS, rhs, lhs });
                            break;
                        case Operator::GREATER_EQUAL:
                            comparisons.push_back(ydl::Comparison { Kind::LESS_EQUAL, rhs, lhs });
                            break;
                    }
                }
                else if constexpr (std::same_as<T, xast::DatalogArithmetic>)
                {
                    using Operator = xast::DatalogArithmeticOperator;
                    using Operation = ydl::Arithmetic::Operation;
                    const auto operation = node.op == Operator::ADD      ? Operation::ADD :
                                           node.op == Operator::SUBTRACT ? Operation::SUBTRACT :
                                           node.op == Operator::MULTIPLY ? Operation::MULTIPLY :
                                           node.op == Operator::DIVIDE   ? Operation::DIVIDE :
                                           node.op == Operator::MIN      ? Operation::MIN :
                                                                           Operation::MAX;
                    const auto lhs = term(node.lhs), rhs = term(node.rhs);
                    arithmetic.push_back(ydl::Arithmetic { operation, lhs, rhs, term(node.result).value });
                }
                else
                    atoms.push_back(atom(node.relation, node.terms, body_relation(node.relation, node.terms.size())).get_index());
            },
            syntax);
    }

    /// (count ?n (condition...)), (sum ?n ?v (condition...)), (min ...), (max ...).
    void aggregate(const xast::DatalogAggregate& syntax, ygg::IndexList<ydl::Aggregate>& aggregates)
    {
        using Function = xast::DatalogAggregateFunction;
        if ((syntax.function == Function::COUNT) == syntax.value.has_value())
            m_context.diagnostics().throw_at(syntax,
                                             InvalidExpressionError("Expected an aggregate: (count ?n (literal...)) or (sum|min|max ?n ?value (literal...))."));
        auto data = ydl::checkout<ydl::Aggregate>(m_datalog_builder);
        data->function = syntax.function == Function::COUNT ? ydl::AggregateFunction::COUNT :
                         syntax.function == Function::SUM   ? ydl::AggregateFunction::SUM :
                         syntax.function == Function::MIN   ? ydl::AggregateFunction::MIN :
                                                              ydl::AggregateFunction::MAX;
        data->result = term(syntax.result).value;
        if (syntax.value)
            data->value = term(*syntax.value).value;
        for (const auto& literal : syntax.body)
            condition(literal, data->atoms, data->negated, data->comparisons, data->arithmetic);
        try
        {
            aggregates.push_back(ydl::insert(datalog(), *data).first.get_index());
        }
        catch (const std::invalid_argument& error)
        {
            m_context.diagnostics().throw_at(syntax, InvalidExpressionError(std::string("Datalog aggregate: ") + error.what()));
        }
    }
};

}  // namespace

DatalogSection::DatalogSection(const DatalogRules* rules,
                               std::set<std::string> read_heads,
                               tyr::formalism::planning::DomainView domain,
                               runir::kr::dl::ConstructorRepository& repository,
                               runir::kr::dl::Builder& builder,
                               FeatureResolver& features,
                               const ext_constructors::ModuleReferences& references,
                               const DiagnosticContext& diagnostics) :
    m_rules(rules),
    m_read_heads(std::move(read_heads)),
    m_domain(domain),
    m_repository(&repository),
    m_builder(&builder),
    m_features(&features),
    m_references(&references),
    m_diagnostics(&diagnostics)
{
    if (!rules)
        return;
    // The heads, their arities and whether they keep least values, so that rules may use heads defined later.
    auto minimizing = std::map<std::string, bool>();
    for (const auto& rule : *rules)
    {
        const auto& name = rule.head.relation;
        const auto clashes = [&]<typename T>(T)
        { return std::ranges::any_of(domain.get_predicates<T>(), [&](auto predicate) { return predicate.get_name() == name.text; }); };
        if (clashes(tyr::formalism::StaticTag {}) || clashes(tyr::formalism::FluentTag {}) || clashes(tyr::formalism::DerivedTag {}))
            diagnostics.throw_at(name, InvalidExpressionError("A Datalog head cannot be named like the predicate " + name.text + "."));
        const auto arity = rule.head.terms.size() + (rule.head.min ? 1 : 0);
        if (const auto [it, inserted] = m_arities.emplace(name.text, arity); !inserted && it->second != arity)
            diagnostics.throw_at(name, ArityMismatchError(name.text, it->second, arity));
        if (const auto [it, inserted] = minimizing.emplace(name.text, rule.head.min.has_value()); !inserted && it->second != rule.head.min.has_value())
            diagnostics.throw_at(name, InvalidExpressionError("Either every rule of " + name.text + " has (min ...) as last head term or none."));
    }
    for (const auto& [name, minimum] : minimizing)
        if (minimum)
            m_weighted.insert(name);
}

void DatalogSection::build(const ast::Identifier& reference)
{
    m_building = true;
    auto builder = ProgramBuilder(m_domain, *m_repository, *m_builder, *m_features, *m_references, m_arities, m_weighted, *m_diagnostics);
    for (const auto& rule : *m_rules)
        builder.add_rule(rule);
    m_program = builder.finish(reference, m_read_heads);
    for (const auto& [name, arity] : m_arities)
        m_heads.emplace(name, builder.derived(name));
    m_building = false;
}

std::optional<HeadReference> DatalogSection::head(const ast::Identifier& name)
{
    if (!m_arities.contains(name.text))
        return std::nullopt;
    if (m_building)
        m_diagnostics->throw_at(
            name,
            InvalidExpressionError("q_head " + name.text + ": a Datalog rule reads a feature that reads the program's heads; define that feature as a rule."));
    if (!m_program)
        build(name);
    YGG_ASSUME(m_program.has_value());  // Set by build().
    return HeadReference { *m_program, m_heads.at(name.text) };
}

void DatalogSection::validate(const ast::Identifier& reference)
{
    if (m_rules && !m_program)
        build(reference);
}

}  // namespace runir::kr::parser::datalog
