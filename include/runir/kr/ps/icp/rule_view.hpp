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
class View<Index<runir::kr::ps::icp::Rule<Kind>>, C>
{
    const C* m_context;
    Index<runir::kr::ps::icp::Rule<Kind>> m_handle;

public:
    View(Index<runir::kr::ps::icp::Rule<Kind>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_source() const noexcept { return make_view(get_data().source, *m_context); }
    auto get_target() const noexcept { return make_view(get_data().target, *m_context); }
    auto get_conditions() const noexcept { return make_view(get_data().conditions, *m_context); }
    auto get_effects() const noexcept { return make_view(get_data().effects, *m_context); }

    auto get_feature() const noexcept
        requires runir::kr::ps::icp::BindingRuleKind<Kind>
    {
        return make_view(get_data().feature, *m_context);
    }

    auto get_register() const noexcept
        requires runir::kr::ps::icp::BindingRuleKind<Kind>
    {
        return make_view(get_data().reg, get_repository(*m_context).get_dl_repository());
    }

    const auto& get_action_name() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    {
        return get_data().action_name;
    }

    const auto& get_argument_names() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    {
        return get_data().argument_names;
    }

    auto get_xconditions() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    {
        return make_view(get_data().xconditions, *m_context);
    }

    auto get_xeffects() const noexcept
        requires std::same_as<Kind, runir::kr::ps::icp::CruleTag>
    {
        return make_view(get_data().xeffects, *m_context);
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}

#endif
