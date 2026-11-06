#ifndef RUNIR_GRAMMAR_ROLE_VIEW_HPP_
#define RUNIR_GRAMMAR_ROLE_VIEW_HPP_

#include "runir/kr/dl/grammar/role_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename Tag, typename V>
    requires runir::kr::dl::RoleConstructorTag<Tag>
struct view_mixin<runir::kr::dl::grammar::Role<Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_identifier() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::RegisterTag> || std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>)
    { return self().data().identifier; }

    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    { return urd::resolve(self().data().predicate, self().context().get_planning_repository()); }

    auto get_polarity() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    { return self().data().polarity; }

    auto get_arg() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::ComplementTag> || std::same_as<Tag, runir::kr::dl::InverseTag>
                 || std::same_as<Tag, runir::kr::dl::TransitiveClosureTag> || std::same_as<Tag, runir::kr::dl::ReflexiveTransitiveClosureTag>
                 || std::same_as<Tag, runir::kr::dl::IdentityTag>)
    { return self().template get<&urd::record_of<V>::arg>(); }

    auto get_lhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::CompositionTag> || std::same_as<Tag, runir::kr::dl::RestrictionTag>)
    { return self().template get<&urd::record_of<V>::lhs>(); }

    auto get_rhs() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::IntersectionTag> || std::same_as<Tag, runir::kr::dl::UnionTag>
                 || std::same_as<Tag, runir::kr::dl::CompositionTag> || std::same_as<Tag, runir::kr::dl::RestrictionTag>)
    { return self().template get<&urd::record_of<V>::rhs>(); }
};
}  // namespace urd

#endif
