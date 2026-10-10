#ifndef RUNIR_KR_PS_ICP_RULE_VIEW_HPP_
#define RUNIR_KR_PS_ICP_RULE_VIEW_HPP_

#include "runir/kr/dl/register_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/icp/memory_state_view.hpp"
#include "runir/kr/ps/icp/rule_data.hpp"
#include "runir/kr/ps/icp/xcondition_view.hpp"
#include "runir/kr/ps/icp/xeffect_view.hpp"

#include <yggdrasil/containers/vector.hpp>

namespace ygg
{

template<runir::kr::ps::icp::RuleKind Kind, typename C>
class View<Index<runir::kr::ps::icp::Rule<Kind>>, C> : public ygg::IndexViewBase<runir::kr::ps::icp::Rule<Kind>, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::icp::Rule<Kind>, C>::IndexViewBase;

    auto get_source() const noexcept { return make_view(this->get_data().source, this->get_context()); }
    auto get_target() const noexcept { return make_view(this->get_data().target, this->get_context()); }
    auto get_conditions() const noexcept { return make_view(this->get_data().conditions, this->get_context()); }
    auto get_effects() const noexcept { return make_view(this->get_data().effects, this->get_context()); }

    auto get_feature() const noexcept
        requires runir::kr::ps::icp::BindingRuleKind<Kind>
    {
        return make_view(this->get_data().feature, this->get_context());
    }

    auto get_register() const noexcept
        requires runir::kr::ps::icp::BindingRuleKind<Kind>
    {
        return make_view(this->get_data().reg, this->get_repository().get_dl_repository());
    }

    const auto& get_action_name() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    {
        return this->get_data().action_name;
    }

    const auto& get_argument_names() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    {
        return this->get_data().argument_names;
    }

    auto get_xconditions() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    {
        return make_view(this->get_data().xconditions, this->get_context());
    }

    auto get_xeffects() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    {
        return make_view(this->get_data().xeffects, this->get_context());
    }
};

}

#endif
