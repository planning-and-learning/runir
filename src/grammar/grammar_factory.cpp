#include "runir/kr/dl/grammar/grammar_factory.hpp"

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/parser.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace runir::kr::dl::grammar
{
namespace
{

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

void add_rule(std::stringstream& out, const std::string& lhs, const std::string& rhs) { out << "    " << lhs << " ::= " << rhs << "\n"; }

std::string non_terminal(const std::string& name) { return "<" + name + ">"; }

std::string non_terminal(const std::string& prefix, const std::string& suffix) { return "<" + prefix + "_" + suffix + ">"; }

std::string non_terminal(const std::string& prefix, bool polarity, const std::string& suffix)
{
    return "<" + prefix + "_" + (polarity ? "true" : "false") + "_" + suffix + ">";
}

std::string category_name(runir::kr::dl::ConceptTag) { return runir::kr::dl::ConceptTag::name; }
std::string category_name(runir::kr::dl::RoleTag) { return runir::kr::dl::RoleTag::name; }
std::string category_name(runir::kr::dl::BooleanTag) { return runir::kr::dl::BooleanTag::name; }
std::string category_name(runir::kr::dl::NumericalTag) { return runir::kr::dl::NumericalTag::name; }

void add_concept_intersection(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::ConceptIntersection::keyword);
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::ConceptIntersection::keyword) + " <concept> <concept>");
}

void add_concept_negation(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::ConceptNegation::keyword);
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::ConceptNegation::keyword) + " <concept>");
}

void add_concept_existential_quantification(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::ConceptExistentialQuantification::keyword);
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::ConceptExistentialQuantification::keyword) + " <role> <concept>");
}

void add_concept_value_restriction(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::ConceptValueRestriction::keyword);
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::ConceptValueRestriction::keyword) + " <role> <concept>");
}

void add_concept_role_value_map_equality(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::ConceptRoleValueMapEquality::keyword);
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::ConceptRoleValueMapEquality::keyword) + " <role> <role>");
}

void add_concept_bot(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::ConceptBot::keyword);
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::ConceptBot::keyword));
}

void add_concept_top(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::ConceptTop::keyword);
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::ConceptTop::keyword));
}

void add_concept_nominal(std::stringstream& out, std::vector<std::string>& heads, std::string_view object_name)
{
    const auto head = non_terminal(ast::ConceptNominal::keyword, std::string(object_name));
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::ConceptNominal::keyword) + " " + quote(object_name));
}

void add_role_transitive_closure(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::RoleTransitiveClosure::keyword);
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::RoleTransitiveClosure::keyword) + " <role_primitive>");
}

void add_role_inverse(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::RoleInverse::keyword);
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::RoleInverse::keyword) + " <role_primitive>");
}

void add_atomic_state(std::stringstream& out, std::vector<std::string>& heads, std::string_view keyword, std::string_view predicate_name)
{
    const auto head = non_terminal(std::string(keyword), std::string(predicate_name));
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(keyword) + " " + quote(predicate_name));
}

void add_boolean_atomic_state(std::stringstream& out, std::vector<std::string>& heads, std::string_view predicate_name)
{
    for (const auto polarity : { true, false })
    {
        const auto head = non_terminal(ast::BooleanAtomicState::keyword, polarity, std::string(predicate_name));
        heads.push_back(head);
        add_rule(out, head, "@" + std::string(ast::BooleanAtomicState::keyword) + " " + quote(predicate_name) + " " + (polarity ? "true" : "false"));
    }
}

void add_atomic_goal(std::stringstream& out, std::vector<std::string>& heads, std::string_view keyword, std::string_view predicate_name)
{
    for (const auto polarity : { true, false })
    {
        const auto head = non_terminal(std::string(keyword), polarity, std::string(predicate_name));
        heads.push_back(head);
        add_rule(out, head, "@" + std::string(keyword) + " " + quote(predicate_name) + " " + (polarity ? "true" : "false"));
    }
}

template<runir::kr::dl::CategoryTag Category>
void add_boolean_nonempty(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::BooleanNonempty::keyword, category_name(Category {}));
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::BooleanNonempty::keyword) + " <" + category_name(Category {}) + ">");
}

template<runir::kr::dl::CategoryTag Category>
void add_numerical_count(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::NumericalCount::keyword, category_name(Category {}));
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::NumericalCount::keyword) + " <" + category_name(Category {}) + ">");
}

void add_numerical_distance(std::stringstream& out, std::vector<std::string>& heads)
{
    const auto head = non_terminal(ast::NumericalDistance::keyword);
    heads.push_back(head);
    add_rule(out, head, "@" + std::string(ast::NumericalDistance::keyword) + " <concept> @role_restriction <role_primitive> <concept_primitive> <concept>");
    add_rule(out, head, "@" + std::string(ast::NumericalDistance::keyword) + " <concept> <role_primitive> <concept>");
}

template<tyr::formalism::FactKind FactKind>
void add_predicate_primitives(tyr::formalism::planning::DomainView domain,
                              std::stringstream& out,
                              std::vector<std::string>& concept_heads,
                              std::vector<std::string>& role_heads,
                              std::vector<std::string>& boolean_heads)
{
    for (const auto predicate : domain.template get_predicates<FactKind>())
    {
        if (predicate.get_arity() == 0)
        {
            add_boolean_atomic_state(out, boolean_heads, predicate.get_name());
        }
        else if (predicate.get_arity() == 1)
        {
            add_atomic_state(out, concept_heads, ast::ConceptAtomicState::keyword, predicate.get_name());
            add_atomic_goal(out, concept_heads, ast::ConceptAtomicGoal::keyword, predicate.get_name());
        }
        else if (predicate.get_arity() == 2)
        {
            add_atomic_state(out, role_heads, ast::RoleAtomicState::keyword, predicate.get_name());
            add_atomic_goal(out, role_heads, ast::RoleAtomicGoal::keyword, predicate.get_name());
        }
    }
}

std::string join(const std::vector<std::string>& values, std::string_view separator)
{
    auto out = std::stringstream {};
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (i > 0)
            out << separator;
        out << values[i];
    }
    return out.str();
}

template<runir::kr::dl::CategoryTag Category>
void add_category_rules(std::stringstream& start_out, std::stringstream& rule_out, std::vector<std::string>& heads, std::vector<std::string>& primitive_heads)
{
    const auto category = category_name(Category {});
    const auto primitive = non_terminal(std::string(category) + "_primitive");

    if (!primitive_heads.empty())
    {
        heads.push_back(primitive);
        add_rule(rule_out, primitive, join(primitive_heads, " | "));
    }

    start_out << "    " << category << " ::= <" << category << "_start>\n";
    add_rule(rule_out, "<" + std::string(category) + "_start>", "<" + std::string(category) + ">");
    add_rule(rule_out, "<" + std::string(category) + ">", join(heads, " | "));
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
    auto primitive_concept_heads = std::vector<std::string> {};
    auto primitive_role_heads = std::vector<std::string> {};
    auto primitive_boolean_heads = std::vector<std::string> {};
    auto primitive_numerical_heads = std::vector<std::string> {};

    auto concept_heads = std::vector<std::string> {};
    auto role_heads = std::vector<std::string> {};
    auto boolean_heads = std::vector<std::string> {};
    auto numerical_heads = std::vector<std::string> {};

    auto rule_out = std::stringstream {};
    rule_out << "[grammar_rules]\n";

    add_concept_bot(rule_out, primitive_concept_heads);
    add_concept_top(rule_out, primitive_concept_heads);

    for (const auto object : domain.get_constants())
        add_concept_nominal(rule_out, primitive_concept_heads, object.get_name());

    add_predicate_primitives<tyr::formalism::StaticTag>(domain, rule_out, primitive_concept_heads, primitive_role_heads, primitive_boolean_heads);
    add_predicate_primitives<tyr::formalism::FluentTag>(domain, rule_out, primitive_concept_heads, primitive_role_heads, primitive_boolean_heads);
    add_predicate_primitives<tyr::formalism::DerivedTag>(domain, rule_out, primitive_concept_heads, primitive_role_heads, primitive_boolean_heads);

    add_concept_intersection(rule_out, concept_heads);
    add_concept_negation(rule_out, concept_heads);
    add_concept_existential_quantification(rule_out, concept_heads);
    add_concept_value_restriction(rule_out, concept_heads);
    add_concept_role_value_map_equality(rule_out, concept_heads);

    add_role_transitive_closure(rule_out, role_heads);
    add_role_inverse(rule_out, role_heads);

    add_boolean_nonempty<runir::kr::dl::ConceptTag>(rule_out, boolean_heads);
    add_boolean_nonempty<runir::kr::dl::RoleTag>(rule_out, boolean_heads);

    add_numerical_count<runir::kr::dl::ConceptTag>(rule_out, numerical_heads);
    add_numerical_count<runir::kr::dl::RoleTag>(rule_out, numerical_heads);
    add_numerical_distance(rule_out, numerical_heads);

    auto start_out = std::stringstream {};
    start_out << "[start_symbols]\n";

    add_category_rules<runir::kr::dl::ConceptTag>(start_out, rule_out, concept_heads, primitive_concept_heads);
    add_category_rules<runir::kr::dl::RoleTag>(start_out, rule_out, role_heads, primitive_role_heads);
    add_category_rules<runir::kr::dl::BooleanTag>(start_out, rule_out, boolean_heads, primitive_boolean_heads);
    add_category_rules<runir::kr::dl::NumericalTag>(start_out, rule_out, numerical_heads, primitive_numerical_heads);

    auto out = std::stringstream {};
    out << start_out.str();
    out << rule_out.str();
    return out.str();
}

}  // namespace runir::kr::dl::grammar
