#include "runir/kr/dl/grammar/grammar_factory.hpp"

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/parser.hpp"

#include <fmt/ranges.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace runir::kr::dl::grammar
{
namespace
{

std::string_view boolean_keyword(bool value) { return value ? runir::kr::dl::TrueTag::keyword : runir::kr::dl::FalseTag::keyword; }

std::string quote(std::string_view value)
{
    auto result = std::string { "\"" };
    for (const auto ch : value)
    {
        if (ch == '"' || ch == '\\')
            result.push_back('\\');
        result.push_back(ch);
    }
    result.push_back('"');
    return result;
}

void add_rule(std::stringstream& out, const std::string& lhs, const std::string& rhs) { out << "    (" << lhs << " (" << rhs << "))\n"; }

std::string category_prefix(runir::kr::dl::ConceptTag) { return "c"; }
std::string category_prefix(runir::kr::dl::RoleTag) { return "r"; }
std::string category_prefix(runir::kr::dl::BooleanTag) { return "b"; }
std::string category_prefix(runir::kr::dl::NumericalTag) { return "n"; }

template<runir::kr::dl::CategoryTag Category>
std::string next_non_terminal(size_t& next_index)
{
    return category_prefix(Category {}) + "_" + std::to_string(next_index++);
}

void add_concept_intersection(std::stringstream& out, std::vector<std::string>& heads, size_t& next_concept, const std::string& concept_)
{
    const auto head = next_non_terminal<runir::kr::dl::ConceptTag>(next_concept);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::ConceptIntersection::keyword) + " " + concept_ + " " + concept_);
}

void add_concept_negation(std::stringstream& out, std::vector<std::string>& heads, size_t& next_concept, const std::string& concept_)
{
    const auto head = next_non_terminal<runir::kr::dl::ConceptTag>(next_concept);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::ConceptNegation::keyword) + " " + concept_);
}

void add_concept_existential_quantification(std::stringstream& out,
                                            std::vector<std::string>& heads,
                                            size_t& next_concept,
                                            const std::string& role,
                                            const std::string& concept_)
{
    const auto head = next_non_terminal<runir::kr::dl::ConceptTag>(next_concept);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::ConceptExistentialQuantification::keyword) + " " + role + " " + concept_);
}

void add_concept_value_restriction(std::stringstream& out,
                                   std::vector<std::string>& heads,
                                   size_t& next_concept,
                                   const std::string& role,
                                   const std::string& concept_)
{
    const auto head = next_non_terminal<runir::kr::dl::ConceptTag>(next_concept);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::ConceptValueRestriction::keyword) + " " + role + " " + concept_);
}

void add_concept_role_value_map_equality(std::stringstream& out, std::vector<std::string>& heads, size_t& next_concept, const std::string& role)
{
    const auto head = next_non_terminal<runir::kr::dl::ConceptTag>(next_concept);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::ConceptRoleValueMapEquality::keyword) + " " + role + " " + role);
}

void add_concept_bot(std::stringstream& out, std::vector<std::string>& heads, size_t& next_concept)
{
    const auto head = next_non_terminal<runir::kr::dl::ConceptTag>(next_concept);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::ConceptBot::keyword));
}

void add_concept_top(std::stringstream& out, std::vector<std::string>& heads, size_t& next_concept)
{
    const auto head = next_non_terminal<runir::kr::dl::ConceptTag>(next_concept);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::ConceptTop::keyword));
}

void add_concept_nominal(std::stringstream& out, std::vector<std::string>& heads, size_t& next_concept, std::string_view object_name)
{
    const auto head = next_non_terminal<runir::kr::dl::ConceptTag>(next_concept);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::ConceptNominal::keyword) + " " + quote(object_name));
}

void add_role_transitive_closure(std::stringstream& out, std::vector<std::string>& heads, size_t& next_role, const std::string& role_primitive)
{
    const auto head = next_non_terminal<runir::kr::dl::RoleTag>(next_role);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::RoleTransitiveClosure::keyword) + " " + role_primitive);
}

void add_role_inverse(std::stringstream& out, std::vector<std::string>& heads, size_t& next_role, const std::string& role_primitive)
{
    const auto head = next_non_terminal<runir::kr::dl::RoleTag>(next_role);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::RoleInverse::keyword) + " " + role_primitive);
}

template<runir::kr::dl::CategoryTag Category>
void add_atomic_state(std::stringstream& out, std::vector<std::string>& heads, size_t& next_index, std::string_view keyword, std::string_view predicate_name)
{
    const auto head = next_non_terminal<Category>(next_index);
    heads.push_back(head);
    add_rule(out, head, std::string(keyword) + " " + quote(predicate_name));
}

void add_boolean_atomic_state(std::stringstream& out, std::vector<std::string>& heads, size_t& next_boolean, std::string_view predicate_name)
{
    for (const auto polarity : { true, false })
    {
        const auto head = next_non_terminal<runir::kr::dl::BooleanTag>(next_boolean);
        heads.push_back(head);
        add_rule(out, head, std::string(ast::BooleanAtomicState::keyword) + " " + quote(predicate_name) + " " + std::string(boolean_keyword(polarity)));
    }
}

template<runir::kr::dl::CategoryTag Category>
void add_atomic_goal(std::stringstream& out, std::vector<std::string>& heads, size_t& next_index, std::string_view keyword, std::string_view predicate_name)
{
    for (const auto polarity : { true, false })
    {
        const auto head = next_non_terminal<Category>(next_index);
        heads.push_back(head);
        add_rule(out, head, std::string(keyword) + " " + quote(predicate_name) + " " + std::string(boolean_keyword(polarity)));
    }
}

void add_boolean_nonempty(std::stringstream& out, std::vector<std::string>& heads, size_t& next_boolean, const std::string& arg)
{
    const auto head = next_non_terminal<runir::kr::dl::BooleanTag>(next_boolean);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::BooleanNonempty::keyword) + " " + arg);
}

void add_numerical_count(std::stringstream& out, std::vector<std::string>& heads, size_t& next_numerical, const std::string& arg)
{
    const auto head = next_non_terminal<runir::kr::dl::NumericalTag>(next_numerical);
    heads.push_back(head);
    add_rule(out, head, std::string(ast::NumericalCount::keyword) + " " + arg);
}

void add_numerical_distance(std::stringstream& out,
                            std::vector<std::string>& heads,
                            size_t& next_numerical,
                            const std::string& concept_,
                            const std::string& role_primitive,
                            const std::string& concept_primitive)
{
    const auto head = next_non_terminal<runir::kr::dl::NumericalTag>(next_numerical);
    heads.push_back(head);
    add_rule(out,
             head,
             std::string(ast::NumericalDistance::keyword) + " " + concept_ + " " + ast::RoleRestriction::keyword + " " + role_primitive + " "
                 + concept_primitive + " " + concept_);
    add_rule(out, head, std::string(ast::NumericalDistance::keyword) + " " + concept_ + " " + role_primitive + " " + concept_);
}

template<tyr::formalism::FactKind FactKind>
void add_predicate_primitives(tyr::formalism::planning::DomainView domain,
                              std::stringstream& out,
                              std::vector<std::string>& concept_heads,
                              std::vector<std::string>& role_heads,
                              std::vector<std::string>& boolean_heads,
                              size_t& next_concept,
                              size_t& next_role,
                              size_t& next_boolean)
{
    for (const auto predicate : domain.template get_predicates<FactKind>())
    {
        if (predicate.get_arity() == 0)
        {
            add_boolean_atomic_state(out, boolean_heads, next_boolean, predicate.get_name());
        }
        else if (predicate.get_arity() == 1)
        {
            add_atomic_state<runir::kr::dl::ConceptTag>(out, concept_heads, next_concept, ast::ConceptAtomicState::keyword, predicate.get_name());
            add_atomic_goal<runir::kr::dl::ConceptTag>(out, concept_heads, next_concept, ast::ConceptAtomicGoal::keyword, predicate.get_name());
        }
        else if (predicate.get_arity() == 2)
        {
            add_atomic_state<runir::kr::dl::RoleTag>(out, role_heads, next_role, ast::RoleAtomicState::keyword, predicate.get_name());
            add_atomic_goal<runir::kr::dl::RoleTag>(out, role_heads, next_role, ast::RoleAtomicGoal::keyword, predicate.get_name());
        }
    }
}

template<runir::kr::dl::CategoryTag Category>
void add_category_rules(std::stringstream& rule_out,
                        const std::string& start,
                        const std::string& category_symbol,
                        const std::string& primitive,
                        std::vector<std::string>& heads,
                        std::vector<std::string>& primitive_heads)
{
    if (!primitive_heads.empty())
    {
        heads.push_back(primitive);
        add_rule(rule_out, primitive, fmt::format("{}", fmt::join(primitive_heads, " or ")));
    }

    add_rule(rule_out, start, category_symbol);
    add_rule(rule_out, category_symbol, fmt::format("{}", fmt::join(heads, " or ")));
}

}  // namespace

GrammarView GrammarFactory::create(GrammarSpecification specification, tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    return parse_grammar(create_description(specification, domain), domain, repository);
}

std::string GrammarFactory::create_description(GrammarSpecification specification, tyr::formalism::planning::DomainView domain)
{
    switch (specification)
    {
        case GrammarSpecification::FRANCE_ET_AL_AAAI2021:
            return create_france_et_al_aaai2021_description(domain);
    }

    throw std::runtime_error("Unknown grammar specification.");
}

GrammarView GrammarFactory::create_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, ConstructorRepository& repository)
{
    return parse_grammar(create_france_et_al_aaai2021_description(domain), domain, repository);
}

std::string GrammarFactory::create_france_et_al_aaai2021_description(tyr::formalism::planning::DomainView domain)
{
    auto next_concept = size_t { 0 };
    auto next_role = size_t { 0 };
    auto next_boolean = size_t { 0 };
    auto next_numerical = size_t { 0 };

    const auto concept_start = next_non_terminal<runir::kr::dl::ConceptTag>(next_concept);
    const auto concept_ = next_non_terminal<runir::kr::dl::ConceptTag>(next_concept);
    const auto concept_primitive = next_non_terminal<runir::kr::dl::ConceptTag>(next_concept);
    const auto role_start = next_non_terminal<runir::kr::dl::RoleTag>(next_role);
    const auto role = next_non_terminal<runir::kr::dl::RoleTag>(next_role);
    const auto role_primitive = next_non_terminal<runir::kr::dl::RoleTag>(next_role);
    const auto boolean_start = next_non_terminal<runir::kr::dl::BooleanTag>(next_boolean);
    const auto boolean = next_non_terminal<runir::kr::dl::BooleanTag>(next_boolean);
    const auto numerical_start = next_non_terminal<runir::kr::dl::NumericalTag>(next_numerical);
    const auto numerical = next_non_terminal<runir::kr::dl::NumericalTag>(next_numerical);

    auto primitive_concept_heads = std::vector<std::string> {};
    auto primitive_role_heads = std::vector<std::string> {};
    auto primitive_boolean_heads = std::vector<std::string> {};
    auto primitive_numerical_heads = std::vector<std::string> {};

    auto concept_heads = std::vector<std::string> {};
    auto role_heads = std::vector<std::string> {};
    auto boolean_heads = std::vector<std::string> {};
    auto numerical_heads = std::vector<std::string> {};

    auto rule_out = std::stringstream {};
    rule_out << "(\n";

    add_concept_bot(rule_out, primitive_concept_heads, next_concept);
    add_concept_top(rule_out, primitive_concept_heads, next_concept);

    for (const auto object : domain.get_constants())
        add_concept_nominal(rule_out, primitive_concept_heads, next_concept, object.get_name());

    add_predicate_primitives<tyr::formalism::StaticTag>(domain,
                                                        rule_out,
                                                        primitive_concept_heads,
                                                        primitive_role_heads,
                                                        primitive_boolean_heads,
                                                        next_concept,
                                                        next_role,
                                                        next_boolean);
    add_predicate_primitives<tyr::formalism::FluentTag>(domain,
                                                        rule_out,
                                                        primitive_concept_heads,
                                                        primitive_role_heads,
                                                        primitive_boolean_heads,
                                                        next_concept,
                                                        next_role,
                                                        next_boolean);
    add_predicate_primitives<tyr::formalism::DerivedTag>(domain,
                                                         rule_out,
                                                         primitive_concept_heads,
                                                         primitive_role_heads,
                                                         primitive_boolean_heads,
                                                         next_concept,
                                                         next_role,
                                                         next_boolean);

    add_concept_intersection(rule_out, concept_heads, next_concept, concept_);
    add_concept_negation(rule_out, concept_heads, next_concept, concept_);
    add_concept_existential_quantification(rule_out, concept_heads, next_concept, role, concept_);
    add_concept_value_restriction(rule_out, concept_heads, next_concept, role, concept_);
    add_concept_role_value_map_equality(rule_out, concept_heads, next_concept, role);

    add_role_transitive_closure(rule_out, role_heads, next_role, role_primitive);
    add_role_inverse(rule_out, role_heads, next_role, role_primitive);

    add_boolean_nonempty(rule_out, boolean_heads, next_boolean, concept_);
    add_boolean_nonempty(rule_out, boolean_heads, next_boolean, role);

    add_numerical_count(rule_out, numerical_heads, next_numerical, concept_);
    add_numerical_count(rule_out, numerical_heads, next_numerical, role);
    add_numerical_distance(rule_out, numerical_heads, next_numerical, concept_, role_primitive, concept_primitive);

    add_category_rules<runir::kr::dl::ConceptTag>(rule_out, concept_start, concept_, concept_primitive, concept_heads, primitive_concept_heads);
    add_category_rules<runir::kr::dl::RoleTag>(rule_out, role_start, role, role_primitive, role_heads, primitive_role_heads);
    add_category_rules<runir::kr::dl::BooleanTag>(rule_out, boolean_start, boolean, std::string(), boolean_heads, primitive_boolean_heads);
    add_category_rules<runir::kr::dl::NumericalTag>(rule_out, numerical_start, numerical, std::string(), numerical_heads, primitive_numerical_heads);

    auto out = std::stringstream {};
    out << rule_out.str();
    out << ")\n";
    return out.str();
}

}  // namespace runir::kr::dl::grammar
