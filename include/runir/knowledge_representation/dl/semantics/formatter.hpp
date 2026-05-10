#ifndef RUNIR_SEMANTICS_FORMATTER_HPP_
#define RUNIR_SEMANTICS_FORMATTER_HPP_

#include "runir/common/config.hpp"
#include "runir/knowledge_representation/dl/grammar/ast/ast.hpp"
#include "runir/knowledge_representation/dl/semantics/constructor_view.hpp"

#include <fmt/format.h>
#include <string>
#include <string_view>

namespace runir::kr::dl::semantics::format
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
std::string concept_constructor(tyr::View<tyr::Index<runir::kr::dl::Concept<Tag>>, C> view)
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
std::string role(tyr::View<tyr::Index<runir::kr::dl::Role<Tag>>, C> view)
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
std::string boolean_constructor(tyr::View<tyr::Index<runir::kr::dl::Boolean<Tag>>, C> view)
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
std::string numerical(tyr::View<tyr::Index<runir::kr::dl::Numerical<Tag>>, C> view)
{
    if constexpr (std::same_as<Tag, runir::kr::dl::CountTag>)
        return fmt::format("@{} {}", runir::kr::dl::grammar::ast::NumericalCount::keyword, variant(view.get_arg()));
    else if constexpr (std::same_as<Tag, runir::kr::dl::DistanceTag>)
        return fmt::format("@{} {} {} {}", runir::kr::dl::grammar::ast::NumericalDistance::keyword, view.get_lhs(), view.get_mid(), view.get_rhs());
}

}  // namespace runir::kr::dl::semantics::format

namespace tyr
{

template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::Concept<Tag>>, C>;
template<runir::kr::dl::RoleConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::Role<Tag>>, C>;
template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::Boolean<Tag>>, C>;
template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
class View<Index<runir::kr::dl::Numerical<Tag>>, C>;
template<runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::Constructor<Category>>, C>;

}  // namespace tyr

#if RUNIR_ENABLE_FMT_FORMATTERS
template<runir::kr::dl::ConceptConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::Concept<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::Concept<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::concept_constructor(view), ctx);
    }
};

template<runir::kr::dl::RoleConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::Role<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::Role<Tag>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::role(view), ctx); }
};

template<runir::kr::dl::BooleanConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::Boolean<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::Boolean<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::boolean_constructor(view), ctx);
    }
};

template<runir::kr::dl::NumericalConstructorTag Tag, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::Numerical<Tag>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::Numerical<Tag>>, C>;
    auto format(View view, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(runir::kr::dl::semantics::format::numerical(view), ctx);
    }
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct fmt::formatter<tyr::View<tyr::Index<runir::kr::dl::Constructor<Category>>, C>> : fmt::formatter<std::string_view>
{
    using View = tyr::View<tyr::Index<runir::kr::dl::Constructor<Category>>, C>;
    auto format(View view, format_context& ctx) const
    {
        const auto text = view.get_variant().apply([](auto arg) { return fmt::format("{}", arg); });
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};
#endif

#endif
