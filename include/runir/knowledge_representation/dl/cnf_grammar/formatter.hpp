#ifndef RUNIR_CNF_GRAMMAR_FORMATTER_HPP_
#define RUNIR_CNF_GRAMMAR_FORMATTER_HPP_

#include "runir/common/config.hpp"
#include "runir/knowledge_representation/dl/cnf_grammar/grammar_view.hpp"
#include "runir/knowledge_representation/dl/cnf_grammar/views.hpp"
#include "runir/knowledge_representation/dl/grammar/ast/ast.hpp"

#include <fmt/format.h>
#include <string>
#include <string_view>

namespace runir::kr::dl::cnf_grammar::format
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

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
std::string concept_constructor(tyr::View<tyr::Index<Concept<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::BotTag>)
        return fmt::format("@{}", runir::kr::dl::grammar::ast::ConceptBot::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return fmt::format("@{}", runir::kr::dl::grammar::ast::ConceptTop::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::ConceptAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::ConceptAtomicGoal::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptIntersection::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptUnion::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::ConceptNegation::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptValueRestriction::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptExistentialQuantification::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapContainmentTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptRoleValueMapContainment::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapEqualityTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::ConceptRoleValueMapEquality::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::ConceptNominal::keyword, quoted(view.get_object().get_name()));
}

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string role(tyr::View<tyr::Index<Role<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return fmt::format("@{}", runir::kr::dl::grammar::ast::RoleUniversal::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::RoleAtomicGoal::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleIntersection::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleUnion::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleComplement::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleInverse::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleComposition::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleTransitiveClosure::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleReflexiveTransitiveClosure::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return fmt::format("@{} {} {}", runir::kr::dl::grammar::ast::RoleRestriction::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::RoleIdentity::keyword, view.get_arg());
}

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string boolean_constructor(tyr::View<tyr::Index<Boolean<Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("@{} {} {}",
                           runir::kr::dl::grammar::ast::BooleanAtomicState::keyword,
                           quoted(view.get_predicate().get_name()),
                           boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::BooleanNonempty::keyword, variant(view.get_arg()));
}

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string numerical(tyr::View<tyr::Index<Numerical<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::NumericalCount::keyword, variant(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("@{} {} {} {}", runir::kr::dl::grammar::ast::NumericalDistance::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string rule(tyr::View<tyr::Index<DerivationRule<Category>>, C> view)
{
    return fmt::format("{} ::= {}", view.get_lhs(), view.get_rhs());
}

template<runir::kr::dl::CategoryTag Category, typename C>
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
    if (auto start = view.template get_start<runir::kr::dl::ConceptTag>())
        text += fmt::format("concept ::= {}\n", *start);
    if (auto start = view.template get_start<runir::kr::dl::RoleTag>())
        text += fmt::format("role ::= {}\n", *start);
    if (auto start = view.template get_start<runir::kr::dl::BooleanTag>())
        text += fmt::format("boolean ::= {}\n", *start);
    if (auto start = view.template get_start<runir::kr::dl::NumericalTag>())
        text += fmt::format("numerical ::= {}\n", *start);

    text += "[grammar_rules]\n";
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::ConceptTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::RoleTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::BooleanTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::NumericalTag>());
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::ConceptTag>());
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::RoleTag>());
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::BooleanTag>());
    append_rules(text, view.template get_substitution_rules<runir::kr::dl::NumericalTag>());
    return text;
}

}  // namespace runir::kr::dl::cnf_grammar::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Concept<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Concept<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::concept_constructor(view), ctx);
    }
};

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Role<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Role<Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::role(view), ctx); }
};

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Boolean<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Boolean<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::boolean_constructor(view), ctx);
    }
};

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Numerical<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Numerical<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::numerical(view), ctx);
    }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Constructor<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::Constructor<Category>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = view.get_variant().apply([](auto arg) { return fmt::format("{}", arg); });
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::NonTerminal<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::NonTerminal<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(view.get_name().str(), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::DerivationRule<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::DerivationRule<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::rule(view), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::SubstitutionRule<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::rule(view), ctx); }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::GrammarTag>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::cnf_grammar::GrammarTag>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::cnf_grammar::format::grammar(view), ctx);
    }
};
#endif

#endif
