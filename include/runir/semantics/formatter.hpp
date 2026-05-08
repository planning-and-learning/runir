#ifndef RUNIR_SEMANTICS_FORMATTER_HPP_
#define RUNIR_SEMANTICS_FORMATTER_HPP_

#include "runir/grammar/ast/ast.hpp"
#include "runir/semantics/constructor_view.hpp"

#include <fmt/format.h>
#include <string>
#include <string_view>

namespace runir::semantics::format
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
std::string concept_constructor(tyr::View<tyr::Index<runir::Concept<Tag>>, C> view)
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
std::string role(tyr::View<tyr::Index<runir::Role<Tag>>, C> view)
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
std::string boolean_constructor(tyr::View<tyr::Index<runir::Boolean<Tag>>, C> view)
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
std::string numerical(tyr::View<tyr::Index<runir::Numerical<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::CountTag>)
        return fmt::format("@{} {}", runir::grammar::ast::NumericalCount::keyword, variant(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::DistanceTag>)
        return fmt::format("@{} {} {} {}", runir::grammar::ast::NumericalDistance::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
}

}  // namespace runir::semantics::format

namespace tyr
{

template<runir::ConceptConstructorTag Tag, typename C>
class View<Index<runir::Concept<Tag>>, C>;
template<runir::RoleConstructorTag Tag, typename C>
class View<Index<runir::Role<Tag>>, C>;
template<runir::BooleanConstructorTag Tag, typename C>
class View<Index<runir::Boolean<Tag>>, C>;
template<runir::NumericalConstructorTag Tag, typename C>
class View<Index<runir::Numerical<Tag>>, C>;
template<runir::CategoryTag Category, typename C>
class View<Index<runir::Constructor<Category>>, C>;

}  // namespace tyr

template<runir::ConceptConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::Concept<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::Concept<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::semantics::format::concept_constructor(view), ctx);
    }
};

template<runir::RoleConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::Role<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::Role<Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::semantics::format::role(view), ctx); }
};

template<runir::BooleanConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::Boolean<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::Boolean<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::semantics::format::boolean_constructor(view), ctx);
    }
};

template<runir::NumericalConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::Numerical<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::Numerical<Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::semantics::format::numerical(view), ctx); }
};

template<runir::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::Constructor<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::Constructor<Category>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = view.get_variant().apply([](auto arg) { return fmt::format("{}", arg); });
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};

#endif
