#ifndef RUNIR_SEMANTICS_BOOLEAN_VIEW_HPP_
#define RUNIR_SEMANTICS_BOOLEAN_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/boolean_data.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename Tag, typename V>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
struct view_mixin<runir::kr::dl::Boolean<Tag>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_predicate() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    { return urd::resolve(self().data().predicate, self().context().get_planning_repository()); }

    auto get_polarity() const noexcept
        requires(runir::kr::dl::is_atomic_state_tag_v<Tag> || runir::kr::dl::is_atomic_goal_tag_v<Tag>)
    { return self().data().polarity; }

    auto get_argument() const noexcept
        requires std::same_as<Tag, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>
    { return self().template get<&urd::record_of<V>::reference>(); }

    auto get_arg() const noexcept
        requires(std::same_as<Tag, runir::kr::dl::NonemptyTag> || std::same_as<Tag, runir::kr::dl::NotTag>)
    { return self().template get<&urd::record_of<V>::arg>(); }

    auto get_lhs() const noexcept
        requires(runir::kr::dl::ComparisonTag<Tag> || runir::kr::dl::LogicalBinaryTag<Tag>)
    { return self().template get<&urd::record_of<V>::lhs>(); }

    auto get_rhs() const noexcept
        requires(runir::kr::dl::ComparisonTag<Tag> || runir::kr::dl::LogicalBinaryTag<Tag>)
    { return self().template get<&urd::record_of<V>::rhs>(); }

    auto get_value() const noexcept
        requires std::same_as<Tag, runir::kr::dl::BooleanConstantTag>
    { return self().data().identifier; }
};
}  // namespace urd

#endif
