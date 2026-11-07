#ifndef RUNIR_GRAMMAR_BOOLEAN_VIEW_HPP_
#define RUNIR_GRAMMAR_BOOLEAN_VIEW_HPP_

#include "runir/kr/dl/grammar/boolean_data.hpp"

#include <concepts>
#include <tuple>
#include <tyr/formalism/predicate_view.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace urd
{
template<typename Tag, typename V>
    requires runir::kr::dl::BaseBooleanConstructorTag<Tag>
struct view_mixin<runir::kr::dl::grammar::Boolean<Tag>, V>
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

    auto get_arg() const noexcept
        requires std::same_as<Tag, runir::kr::dl::NonemptyTag>
    { return self().template get<&urd::record_of<V>::arg>(); }
};
}  // namespace urd

#endif
