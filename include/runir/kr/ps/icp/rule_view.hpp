#ifndef RUNIR_KR_PS_ICP_RULE_VIEW_HPP_
#define RUNIR_KR_PS_ICP_RULE_VIEW_HPP_

#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/icp/memory_state_view.hpp"
#include "runir/kr/ps/icp/rule_data.hpp"
#include "runir/kr/ps/icp/xcondition_view.hpp"
#include "runir/kr/ps/icp/xeffect_view.hpp"

#include <urd/elements/flat.hpp>

namespace urd
{
template<runir::kr::ps::icp::RuleKind Kind, typename V>
struct view_mixin<runir::kr::ps::icp::Rule<Kind>, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_source() const noexcept { return self().template get<&urd::record_of<V>::source>(); }
    auto get_target() const noexcept { return self().template get<&urd::record_of<V>::target>(); }
    auto get_conditions() const noexcept { return self().template get<&urd::record_of<V>::conditions>(); }
    auto get_effects() const noexcept { return self().template get<&urd::record_of<V>::effects>(); }

    auto get_feature() const noexcept
        requires runir::kr::ps::icp::BindingRuleKind<Kind>
    { return self().template get<&urd::record_of<V>::feature>(); }

    auto get_register() const noexcept
        requires runir::kr::ps::icp::BindingRuleKind<Kind>
    { return urd::resolve(self().data().reg, get_repository(self().context()).get_dl_repository()); }

    const auto& get_action_name() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    { return self().data().action_name; }

    const auto& get_argument_names() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    { return self().data().argument_names; }

    auto get_xconditions() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    { return self().template get<&urd::record_of<V>::xconditions>(); }

    auto get_xeffects() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    { return self().template get<&urd::record_of<V>::xeffects>(); }
};
}  // namespace urd

#endif
