#ifndef RUNIR_CNF_GRAMMAR_FORMATTER_HPP_
#define RUNIR_CNF_GRAMMAR_FORMATTER_HPP_

#include "runir/cnf_grammar/grammar_view.hpp"
#include "runir/cnf_grammar/views.hpp"
#include "runir/grammar/ast/ast.hpp"

#include <fmt/format.h>
#include <string>
#include <string_view>

namespace runir::cnf_grammar::format
{

template<typename String>
std::string quoted(const String& value)
{
    return fmt::format("\"{}\"", value.str());
}

inline std::string boolean(bool value) { return value ? "true" : "false"; }

template<typename View>
std::string variant(View view)
{
    return view.apply([](auto arg) { return fmt::format("{}", arg); });
}

template<runir::ConceptConstructorTag Tag, typename C>
std::string concept_constructor(tyr::View<tyr::Index<Concept<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::BotTag>)
        return fmt::format("@{}", runir::grammar::ast::ConceptBot::keyword);
    else if constexpr (std::same_as<Tag, runir::TopTag>)
        return fmt::format("@{}", runir::grammar::ast::ConceptTop::keyword);
    else if constexpr (runir::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {}", runir::grammar::ast::ConceptAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::is_atomic_goal_tag_v<Tag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::ConceptAtomicGoal::keyword, quoted(view.get_predicate().get_name()), boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::IntersectionTag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::ConceptIntersection::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::UnionTag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::ConceptUnion::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::NegationTag>)
        return fmt::format("@{} {}", runir::grammar::ast::ConceptNegation::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::ValueRestrictionTag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::ConceptValueRestriction::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::ExistentialQuantificationTag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::ConceptExistentialQuantification::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::RoleValueMapContainmentTag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::ConceptRoleValueMapContainment::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::RoleValueMapEqualityTag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::ConceptRoleValueMapEquality::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::NominalTag>)
        return fmt::format("@{} {}", runir::grammar::ast::ConceptNominal::keyword, quoted(view.get_object().get_name()));
}

template<runir::RoleConstructorTag Tag, typename C>
std::string role(tyr::View<tyr::Index<Role<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::UniversalTag>)
        return fmt::format("@{}", runir::grammar::ast::RoleUniversal::keyword);
    else if constexpr (runir::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {}", runir::grammar::ast::RoleAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::is_atomic_goal_tag_v<Tag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::RoleAtomicGoal::keyword, quoted(view.get_predicate().get_name()), boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::IntersectionTag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::RoleIntersection::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::UnionTag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::RoleUnion::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::ComplementTag>)
        return fmt::format("@{} {}", runir::grammar::ast::RoleComplement::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::InverseTag>)
        return fmt::format("@{} {}", runir::grammar::ast::RoleInverse::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::CompositionTag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::RoleComposition::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::TransitiveClosureTag>)
        return fmt::format("@{} {}", runir::grammar::ast::RoleTransitiveClosure::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::ReflexiveTransitiveClosureTag>)
        return fmt::format("@{} {}", runir::grammar::ast::RoleReflexiveTransitiveClosure::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::RestrictionTag>)
        return fmt::format("@{} {} {}", runir::grammar::ast::RoleRestriction::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::IdentityTag>)
        return fmt::format("@{} {}", runir::grammar::ast::RoleIdentity::keyword, view.get_arg());
}

template<runir::BooleanConstructorTag Tag, typename C>
std::string boolean_constructor(tyr::View<tyr::Index<Boolean<Tag>>, C> view)
{
    if constexpr (runir::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::grammar::ast::BooleanAtomicState::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::NonemptyTag>)
        return fmt::format("@{} {}", runir::grammar::ast::BooleanNonempty::keyword, variant(view.get_arg()));
}

template<runir::NumericalConstructorTag Tag, typename C>
std::string numerical(tyr::View<tyr::Index<Numerical<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::CountTag>)
        return fmt::format("@{} {}", runir::grammar::ast::NumericalCount::keyword, variant(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::DistanceTag>)
        return fmt::format("@{} {} {} {}", runir::grammar::ast::NumericalDistance::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
}

template<runir::CategoryTag Category, typename C>
std::string rule(tyr::View<tyr::Index<DerivationRule<Category>>, C> view)
{
    return fmt::format("{} ::= {}", view.get_lhs(), view.get_rhs());
}

template<runir::CategoryTag Category, typename C>
std::string rule(tyr::View<tyr::Index<SubstitutionRule<Category>>, C> view)
{
    return fmt::format("{} ::= {}", view.get_lhs(), view.get_rhs());
}

template<typename Rules>
void append_rules(std::string& text, Rules rules)
{
    for (auto rule : rules)
        text += fmt::format("{}\n", rule);
}

template<typename View>
std::string grammar(View view)
{
    auto text = std::string("[start_symbols]\n");
    if (auto start = view.template get_start<runir::ConceptTag>())
        text += fmt::format("concept ::= {}\n", *start);
    if (auto start = view.template get_start<runir::RoleTag>())
        text += fmt::format("role ::= {}\n", *start);
    if (auto start = view.template get_start<runir::BooleanTag>())
        text += fmt::format("boolean ::= {}\n", *start);
    if (auto start = view.template get_start<runir::NumericalTag>())
        text += fmt::format("numerical ::= {}\n", *start);

    text += "[grammar_rules]\n";
    append_rules(text, view.template get_derivation_rules<runir::ConceptTag>());
    append_rules(text, view.template get_derivation_rules<runir::RoleTag>());
    append_rules(text, view.template get_derivation_rules<runir::BooleanTag>());
    append_rules(text, view.template get_derivation_rules<runir::NumericalTag>());
    append_rules(text, view.template get_substitution_rules<runir::ConceptTag>());
    append_rules(text, view.template get_substitution_rules<runir::RoleTag>());
    append_rules(text, view.template get_substitution_rules<runir::BooleanTag>());
    append_rules(text, view.template get_substitution_rules<runir::NumericalTag>());
    return text;
}

}  // namespace runir::cnf_grammar::format

template<runir::ConceptConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::cnf_grammar::Concept<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::cnf_grammar::Concept<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::cnf_grammar::format::concept_constructor(view), ctx);
    }
};

template<runir::RoleConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::cnf_grammar::Role<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::cnf_grammar::Role<Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::cnf_grammar::format::role(view), ctx); }
};

template<runir::BooleanConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::cnf_grammar::Boolean<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::cnf_grammar::Boolean<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::cnf_grammar::format::boolean_constructor(view), ctx);
    }
};

template<runir::NumericalConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::cnf_grammar::Numerical<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::cnf_grammar::Numerical<Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::cnf_grammar::format::numerical(view), ctx); }
};

template<runir::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::cnf_grammar::Constructor<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::cnf_grammar::Constructor<Category>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = view.get_variant().apply([](auto arg) { return fmt::format("{}", arg); });
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::cnf_grammar::NonTerminal<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::cnf_grammar::NonTerminal<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(view.get_name().str(), ctx); }
};

template<runir::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::cnf_grammar::DerivationRule<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::cnf_grammar::DerivationRule<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::cnf_grammar::format::rule(view), ctx); }
};

template<runir::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::cnf_grammar::SubstitutionRule<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::cnf_grammar::SubstitutionRule<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::cnf_grammar::format::rule(view), ctx); }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::cnf_grammar::GrammarTag>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::cnf_grammar::GrammarTag>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::cnf_grammar::format::grammar(view), ctx); }
};

#endif
