#ifndef RUNIR_GRAMMAR_FORMATTER_HPP_
#define RUNIR_GRAMMAR_FORMATTER_HPP_

#include "runir/common/config.hpp"
#include "runir/kr/dl/grammar/ast/ast.hpp"
#include "runir/kr/dl/grammar/grammar_view.hpp"
#include "runir/kr/dl/grammar/views.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <string_view>
#include <vector>

namespace runir::kr::dl::grammar::format
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
        return fmt::format("{}", ast::ConceptBot::keyword);
    else if constexpr (std::same_as<Tag, runir::kr::dl::TopTag>)
        return fmt::format("{}", ast::ConceptTop::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}", ast::ConceptAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}", ast::ConceptAtomicGoal::keyword, quoted(view.get_predicate().get_name()), boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}", ast::ConceptIntersection::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}", ast::ConceptUnion::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NegationTag>)
        return fmt::format("{} {}", ast::ConceptNegation::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ValueRestrictionTag>)
        return fmt::format("{} {} {}", ast::ConceptValueRestriction::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ExistentialQuantificationTag>)
        return fmt::format("{} {} {}", ast::ConceptExistentialQuantification::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapContainmentTag>)
        return fmt::format("{} {} {}", ast::ConceptRoleValueMapContainment::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RoleValueMapEqualityTag>)
        return fmt::format("{} {} {}", ast::ConceptRoleValueMapEquality::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::NominalTag>)
        return fmt::format("{} {}", ast::ConceptNominal::keyword, quoted(view.get_object().get_name()));
}

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string role(tyr::View<tyr::Index<Role<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::UniversalTag>)
        return fmt::format("{}", ast::RoleUniversal::keyword);
    else if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {}", ast::RoleAtomicState::keyword, quoted(view.get_predicate().get_name()));
    else if constexpr (runir::kr::dl::is_atomic_goal_tag_v<Tag>)
        return fmt::format("{} {} {}", ast::RoleAtomicGoal::keyword, quoted(view.get_predicate().get_name()), boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::IntersectionTag>)
        return fmt::format("{} {} {}", ast::RoleIntersection::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::UnionTag>)
        return fmt::format("{} {} {}", ast::RoleUnion::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ComplementTag>)
        return fmt::format("{} {}", ast::RoleComplement::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::InverseTag>)
        return fmt::format("{} {}", ast::RoleInverse::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::CompositionTag>)
        return fmt::format("{} {} {}", ast::RoleComposition::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::TransitiveClosureTag>)
        return fmt::format("{} {}", ast::RoleTransitiveClosure::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>)
        return fmt::format("{} {}", ast::RoleReflexiveTransitiveClosure::keyword, view.get_arg());
    else if constexpr (std::same_as<Tag, runir::kr::dl::RestrictionTag>)
        return fmt::format("{} {} {}", ast::RoleRestriction::keyword, view.get_lhs(), view.get_rhs());
    else if constexpr (std::same_as<Tag, runir::kr::dl::IdentityTag>)
        return fmt::format("{} {}", ast::RoleIdentity::keyword, view.get_arg());
}

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string boolean_constructor(tyr::View<tyr::Index<Boolean<Tag>>, C> view)
{
    if constexpr (runir::kr::dl::is_atomic_state_tag_v<Tag>)
        return fmt::format("{} {} {}", ast::BooleanAtomicState::keyword, quoted(view.get_predicate().get_name()), boolean(view.get_polarity()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::NonemptyTag>)
        return fmt::format("{} {}", ast::BooleanNonempty::keyword, variant(view.get_arg()));
}

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string numerical(tyr::View<tyr::Index<Numerical<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("{} {}", ast::NumericalCount::keyword, variant(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("{} {} {} {}", ast::NumericalDistance::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
}

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<Concept<Tag>>, C> view)
{
    return concept_constructor(view);
}

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<Role<Tag>>, C> view)
{
    return role(view);
}

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<Boolean<Tag>>, C> view)
{
    return boolean_constructor(view);
}

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
std::string constructor_body(tyr::View<tyr::Index<Numerical<Tag>>, C> view)
{
    return numerical(view);
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string constructor_body(tyr::View<tyr::Index<Constructor<Category>>, C> view)
{
    return view.get_variant().apply([](auto arg) { return constructor_body(arg); });
}

template<runir::kr::dl::CategoryTag Category, typename C>
std::string rule(tyr::View<tyr::Index<DerivationRule<Category>>, C> view)
{
    auto rhs = std::vector<std::string> {};
    for (auto symbol : view.get_rhs())
    {
        rhs.push_back(symbol.get_variant().apply(
            [](auto arg)
            {
                if constexpr (std::same_as<std::decay_t<decltype(arg)>, tyr::View<tyr::Index<Constructor<Category>>, C>>)
                    return constructor_body(arg);
                else
                    return fmt::format("{}", arg);
            }));
    }

    return fmt::format("({} ({}))", view.get_lhs(), fmt::join(rhs, " or "));
}

template<typename Rules>
void append_rules(std::string& text, Rules rules)
{
    for (auto rule : rules)
        text += fmt::format("    {}\n", rule);
}

template<typename View>
std::string grammar(View view)
{
    auto text = std::string("(\n");
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::ConceptTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::RoleTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::BooleanTag>());
    append_rules(text, view.template get_derivation_rules<runir::kr::dl::NumericalTag>());
    text += ")\n";
    return text;
}

}  // namespace runir::kr::dl::grammar::format

#if RUNIR_ENABLE_FMT_FORMATTERS
template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::Concept<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::Concept<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::grammar::format::concept_constructor(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::Role<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::Role<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::grammar::format::role(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::Boolean<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::Boolean<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::grammar::format::boolean_constructor(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::Numerical<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::Numerical<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::grammar::format::numerical(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::Constructor<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::Constructor<Category>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = fmt::format("({})", runir::kr::dl::grammar::format::constructor_body(view));
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::NonTerminal<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::NonTerminal<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(view.get_name().str(), ctx); }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::ConstructorOrNonTerminal<Category>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = view.get_variant().apply([](auto arg) { return fmt::format("{}", arg); });
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::DerivationRule<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::DerivationRule<Category>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::grammar::format::rule(view), ctx); }
};

template<typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::grammar::GrammarTag>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::grammar::GrammarTag>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::grammar::format::grammar(view), ctx); }
};
#endif

#endif
