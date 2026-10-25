#include "kr/parser/datalog.hpp"

#include "kr/parser/resolution.hpp"
#include "runir/kr/dl/detail/repository_instantiations.hpp"
#include "runir/kr/errors.hpp"

#include <algorithm>
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

/// The items of a list, or nullptr for an atom.
const ast::SList* as_list(const ast::SExpression& expression)
{
    if (const auto* list = boost::get<boost::spirit::x3::forward_ast<ast::SList>>(&expression))
        return &list->get();
    return nullptr;
}

const ast::Identifier* as_atom(const ast::SExpression& expression) { return boost::get<ast::Identifier>(&expression); }

/// The head atom or a literal: (name term...), its name and terms.
struct AtomSyntax
{
    const ast::Identifier* name;
    std::vector<const ast::Identifier*> terms;
};

class ProgramBuilder
{
public:
    ProgramBuilder(tyr::formalism::planning::DomainView domain,
                   dl::ConstructorRepository& repository,
                   dl::Builder& builder,
                   FeatureResolver& features,
                   const std::map<std::string, size_t>& arities,
                   const DiagnosticContext& diagnostics) :
        m_domain(domain),
        m_repository(repository),
        m_builder(builder),
        m_features(features),
        m_arities(arities),
        m_diagnostics(diagnostics)
    {
        for (const auto& [name, arity] : arities)
        {
            auto data = ydl::checkout<ydl::Relation>(m_datalog_builder);
            data->id = m_relation_names.size();
            data->arity = arity;
            m_derived.emplace(name, ydl::insert(datalog(), *data).first.get_index());
            m_relation_names.push_back(name);
        }
    }

    void add_rule(const ast::SExpression& expression)
    {
        const auto* list = as_list(expression);
        const auto* keyword = list && !list->items.empty() ? as_atom(list->items.front()) : nullptr;
        if (!keyword || keyword->text != ":rule" || list->items.size() < 2)
            m_diagnostics.throw_at(expression, InvalidExpressionError("Expected a Datalog rule: (:rule (head ?x...) literal...)."));
        m_variables.clear();
        auto rule = ydl::checkout<ydl::Rule>(m_datalog_builder);
        const auto head = atom_syntax(list->items[1]);
        rule->head = atom(head, head_relation(head)).get_index();
        for (size_t i = 2; i < list->items.size(); ++i)
            literal(list->items[i], rule->atoms, rule->negated, rule->comparisons, &rule->aggregates);
        try
        {
            m_rules.push_back(ydl::insert(datalog(), *rule).first.get_index());
        }
        catch (const std::invalid_argument& error)
        {
            m_diagnostics.throw_at(expression, InvalidExpressionError(std::string("Datalog rule: ") + error.what()));
        }
    }

    ygg::Index<dl::DatalogProgram> finish(const ast::Identifier& reference)
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
            m_diagnostics.throw_at(reference, InvalidExpressionError(std::string("Datalog program: ") + error.what()));
        }
        auto data = dl::checkout<dl::DatalogProgram>(m_builder);
        data->program = view.get_index();
        for (const auto input : m_inputs)
            data->inputs.push_back(input);
        for (const auto& name : m_relation_names)
            data->relation_names.emplace_back(name);
        for (const auto& name : m_input_names)
            data->input_names.emplace_back(name);
        return dl::insert(m_repository, *data).first.get_index();
    }

    ygg::Index<ydl::Relation> derived(const std::string& name) const { return m_derived.at(name); }

private:
    tyr::formalism::planning::DomainView m_domain;
    dl::ConstructorRepository& m_repository;
    dl::Builder& m_builder;
    FeatureResolver& m_features;
    const std::map<std::string, size_t>& m_arities;
    const DiagnosticContext& m_diagnostics;
    ydl::DatalogBuilder m_datalog_builder;
    std::map<std::string, ygg::Index<ydl::Relation>> m_derived;
    std::vector<std::string> m_relation_names;
    std::vector<ygg::Index<dl::Query<>>> m_inputs;
    std::vector<std::string> m_input_names;
    std::map<ygg::uint_t, ygg::Index<ydl::Relation>> m_input_relations;  ///< By input query index.
    std::map<std::string, ygg::uint_t> m_variables;
    std::vector<ygg::Index<ydl::Rule>> m_rules;

    ydl::DatalogRepository& datalog() { return m_repository.get_datalog_repository(); }

    AtomSyntax atom_syntax(const ast::SExpression& expression) const
    {
        const auto* list = as_list(expression);
        const auto* name = list && !list->items.empty() ? as_atom(list->items.front()) : nullptr;
        if (!name)
            m_diagnostics.throw_at(expression, InvalidExpressionError("Expected an atom: (relation term...)."));
        auto result = AtomSyntax { name, {} };
        for (size_t i = 1; i < list->items.size(); ++i)
        {
            const auto* term = as_atom(list->items[i]);
            if (!term)
                m_diagnostics.throw_at(list->items[i], InvalidExpressionError("Expected a term: ?variable or object name."));
            result.terms.push_back(term);
        }
        return result;
    }

    ydl::Term term(const ast::Identifier& name)
    {
        if (name.text.starts_with('?'))
            return ydl::Term::variable(m_variables.try_emplace(name.text, ygg::to_uint_t(m_variables.size())).first->second);
        for (const auto object : m_domain.get_constants())
            if (object.get_name() == name.text)
                return ydl::Term::constant(static_cast<ygg::uint_t>(object.get_index()));
        m_diagnostics.throw_at(name, UndefinedSymbolError("constant", name.text));
    }

    ydl::AtomView atom(const AtomSyntax& syntax, ygg::Index<ydl::Relation> relation)
    {
        auto data = ydl::checkout<ydl::Atom>(m_datalog_builder);
        data->relation = relation;
        for (const auto* t : syntax.terms)
            data->terms.push_back(term(*t));
        try
        {
            return ydl::insert(datalog(), *data).first;
        }
        catch (const std::invalid_argument&)
        {
            m_diagnostics.throw_at(*syntax.name,
                                   ArityMismatchError(syntax.name->text, ydl::RelationView(relation, datalog()).get_arity(), syntax.terms.size()));
        }
    }

    ygg::Index<ydl::Relation> head_relation(const AtomSyntax& syntax)
    {
        const auto it = m_derived.find(syntax.name->text);
        if (it == m_derived.end())
            m_diagnostics.throw_at(*syntax.name, InvalidExpressionError("Not a Datalog head: " + syntax.name->text));
        return it->second;
    }

    ygg::Index<dl::QueryColumn> column(size_t position)
    {
        auto data = dl::checkout<dl::QueryColumn>(m_builder);
        data->name = "_" + std::to_string(position);
        return dl::insert(m_repository, *data).first.get_index();
    }
    ygg::IndexList<dl::QueryColumn> columns(size_t arity)
    {
        auto result = ygg::IndexList<dl::QueryColumn>();
        for (size_t i = 0; i < arity; ++i)
            result.push_back(column(i));
        return result;
    }
    template<typename Tag>
    ygg::Index<dl::Query<>> query(ygg::Index<dl::Query<Tag>> index)
    {
        auto data = dl::checkout<dl::Query<>>(m_builder);
        data->variant = index;
        return dl::insert(m_repository, *data).first.get_index();
    }

    /// The input relation of an input query, a new input slot for a new query.
    ygg::Index<ydl::Relation> input(ygg::Index<dl::Query<>> query, std::string name)
    {
        if (const auto it = m_input_relations.find(query.get_value()); it != m_input_relations.end())
            return it->second;
        auto data = ydl::checkout<ydl::Relation>(m_datalog_builder);
        data->id = m_inputs.size();
        data->arity = ygg::make_view(query, m_repository).get_schema().size();
        data->input_slot = m_inputs.size();
        const auto relation = ydl::insert(datalog(), *data).first.get_index();
        m_inputs.push_back(query);
        m_input_names.push_back(std::move(name));
        m_input_relations.emplace(query.get_value(), relation);
        return relation;
    }

    /// A predicate's atoms in the state, or in the goal.
    ygg::Index<ydl::Relation> predicate(const ast::Identifier& name, size_t arity, bool goal)
    {
        const auto make = [&]<typename T>(T, auto predicate) -> ygg::Index<dl::Query<>>
        {
            if (goal)
            {
                auto data = dl::checkout<dl::Query<dl::AtomicGoalTag<T>>>(m_builder);
                data->predicate = predicate;
                data->polarity = true;
                data->columns = columns(arity);
                return query(dl::insert(m_repository, *data).first.get_index());
            }
            auto data = dl::checkout<dl::Query<dl::AtomicStateTag<T>>>(m_builder);
            data->predicate = predicate;
            data->columns = columns(arity);
            return query(dl::insert(m_repository, *data).first.get_index());
        };
        const auto index = runir::kr::parser::resolve_predicate(m_domain, name, arity, goal ? "goal atom" : "atom", m_diagnostics, make);
        return input(index, (goal ? "goal " : "") + name.text);
    }

    /// The relation of a body atom: a head, a feature, or a domain predicate.
    ygg::Index<ydl::Relation> body_relation(const AtomSyntax& syntax)
    {
        const auto& name = *syntax.name;
        if (const auto it = m_derived.find(name.text); it != m_derived.end())
            return it->second;
        if (const auto feature = m_features.resolve(name))
        {
            const auto index = std::visit(
                [&]<typename T>(ygg::Index<T> resolved) -> ygg::Index<dl::Query<>>
                {
                    if constexpr (std::same_as<T, dl::Constructor<dl::ConceptTag>>)
                    {
                        auto data = dl::checkout<dl::Query<dl::QueryConceptTag>>(m_builder);
                        data->arg = resolved;
                        data->columns = columns(1);
                        return query(dl::insert(m_repository, *data).first.get_index());
                    }
                    else if constexpr (std::same_as<T, dl::Constructor<dl::RoleTag>>)
                    {
                        auto data = dl::checkout<dl::Query<dl::QueryRoleTag>>(m_builder);
                        data->arg = resolved;
                        data->columns = columns(2);
                        return query(dl::insert(m_repository, *data).first.get_index());
                    }
                    else if constexpr (std::same_as<T, dl::Query<>>)
                        return resolved;
                    else
                        m_diagnostics.throw_at(name,
                                               InvalidExpressionError("A Datalog rule can read concept, role and query features, not " + name.text + "."));
                },
                *feature);
            return input(index, name.text);
        }
        return predicate(name, syntax.terms.size(), false);
    }

    /// Adds a body literal: an atom, a goal atom, a negated atom, a comparison, or (with aggregates) an aggregate.
    void literal(const ast::SExpression& expression,
                 ygg::IndexList<ydl::Atom>& atoms,
                 ygg::IndexList<ydl::Atom>& negated,
                 cista::offset::vector<ydl::Comparison>& comparisons,
                 ygg::IndexList<ydl::Aggregate>* aggregates)
    {
        const auto* list = as_list(expression);
        const auto* first = list && !list->items.empty() ? as_atom(list->items.front()) : nullptr;
        if (!first)
            m_diagnostics.throw_at(expression, InvalidExpressionError("Expected a Datalog literal."));
        const auto& keyword = first->text;
        const auto items = list->items.size();
        if (keyword == "not" && items == 2)
        {
            const auto syntax = atom_syntax(list->items[1]);
            negated.push_back(atom(syntax, body_relation(syntax)).get_index());
        }
        else if ((keyword == "=" || keyword == "!=") && items == 3)
        {
            const auto* lhs = as_atom(list->items[1]);
            const auto* rhs = as_atom(list->items[2]);
            if (!lhs || !rhs)
                m_diagnostics.throw_at(expression, InvalidExpressionError("A comparison compares two terms."));
            comparisons.push_back(ydl::Comparison { keyword == "=" ? ydl::Comparison::Kind::EQUAL : ydl::Comparison::Kind::NOT_EQUAL, term(*lhs), term(*rhs) });
        }
        else if (keyword == "goal")
        {
            auto syntax = atom_syntax(expression);
            if (syntax.terms.empty())
                m_diagnostics.throw_at(expression, InvalidExpressionError("Expected a goal atom: (goal predicate term...)."));
            const auto* predicate_name = syntax.terms.front();
            syntax.name = predicate_name;
            syntax.terms.erase(syntax.terms.begin());
            atoms.push_back(atom(syntax, predicate(*predicate_name, syntax.terms.size(), true)).get_index());
        }
        else if (keyword == "count" || keyword == "sum" || keyword == "min" || keyword == "max")
            atoms_or_aggregate(expression, *list, keyword, aggregates);
        else
        {
            const auto syntax = atom_syntax(expression);
            atoms.push_back(atom(syntax, body_relation(syntax)).get_index());
        }
    }

    /// (count ?n (literal...)), (sum ?n ?v (literal...)), (min ...), (max ...).
    void atoms_or_aggregate(const ast::SExpression& expression, const ast::SList& list, const std::string& function, ygg::IndexList<ydl::Aggregate>* aggregates)
    {
        const auto valued = function != "count";
        const auto* result = list.items.size() > 1 ? as_atom(list.items[1]) : nullptr;
        const auto* value = valued && list.items.size() > 2 ? as_atom(list.items[2]) : nullptr;
        const auto* body = list.items.size() == (valued ? 4u : 3u) ? as_list(list.items.back()) : nullptr;
        if (!aggregates)
            m_diagnostics.throw_at(expression, InvalidExpressionError("Aggregates cannot be nested."));
        if (!result || !result->text.starts_with('?') || (valued && (!value || !value->text.starts_with('?'))) || !body)
            m_diagnostics.throw_at(expression,
                                   InvalidExpressionError("Expected an aggregate: (count ?n (literal...)) or (" + function + " ?n ?value (literal...))."));
        auto data = ydl::checkout<ydl::Aggregate>(m_datalog_builder);
        data->function = function == "count" ? ydl::AggregateFunction::COUNT :
                         function == "sum"   ? ydl::AggregateFunction::SUM :
                         function == "min"   ? ydl::AggregateFunction::MIN :
                                               ydl::AggregateFunction::MAX;
        data->result = term(*result).value;
        if (valued)
            data->value = term(*value).value;
        for (const auto& item : body->items)
            literal(item, data->atoms, data->negated, data->comparisons, nullptr);
        try
        {
            aggregates->push_back(ydl::insert(datalog(), *data).first.get_index());
        }
        catch (const std::invalid_argument& error)
        {
            m_diagnostics.throw_at(expression, InvalidExpressionError(std::string("Datalog aggregate: ") + error.what()));
        }
    }
};

}  // namespace

DatalogSection::DatalogSection(const ast::DatalogSection* section,
                               tyr::formalism::planning::DomainView domain,
                               runir::kr::dl::ConstructorRepository& repository,
                               runir::kr::dl::Builder& builder,
                               FeatureResolver& features,
                               const DiagnosticContext& diagnostics) :
    m_section(section),
    m_domain(domain),
    m_repository(&repository),
    m_builder(&builder),
    m_features(&features),
    m_diagnostics(&diagnostics)
{
    if (!section)
        return;
    // The heads and their arities, so that rules may use heads defined later.
    for (const auto& rule : section->rules)
    {
        const auto* list = as_list(rule);
        const auto* keyword = list && list->items.size() >= 2 ? as_atom(list->items.front()) : nullptr;
        const auto* head = keyword && keyword->text == ":rule" ? as_list(list->items[1]) : nullptr;
        const auto* name = head && !head->items.empty() ? as_atom(head->items.front()) : nullptr;
        if (!name)
            diagnostics.throw_at(rule, InvalidExpressionError("Expected a Datalog rule: (:rule (head ?x...) literal...)."));
        const auto clashes = [&]<typename T>(T)
        { return std::ranges::any_of(domain.get_predicates<T>(), [&](auto predicate) { return predicate.get_name() == name->text; }); };
        if (clashes(tyr::formalism::StaticTag {}) || clashes(tyr::formalism::FluentTag {}) || clashes(tyr::formalism::DerivedTag {}))
            diagnostics.throw_at(*name, InvalidExpressionError("A Datalog head cannot be named like the predicate " + name->text + "."));
        const auto arity = head->items.size() - 1;
        if (const auto [it, inserted] = m_arities.emplace(name->text, arity); !inserted && it->second != arity)
            diagnostics.throw_at(*name, ArityMismatchError(name->text, it->second, arity));
    }
}

void DatalogSection::build(const ast::Identifier& reference)
{
    m_building = true;
    auto builder = ProgramBuilder(m_domain, *m_repository, *m_builder, *m_features, m_arities, *m_diagnostics);
    for (const auto& rule : m_section->rules)
        builder.add_rule(rule);
    m_program = builder.finish(reference);
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

FeatureScope make_scope(FeatureResolver& features, DatalogSection& section)
{
    return FeatureScope { [&features](const ast::Identifier& name) { return features.resolve(name); },
                          [&section](const ast::Identifier& name) { return section.head(name); } };
}

}  // namespace runir::kr::parser::datalog
