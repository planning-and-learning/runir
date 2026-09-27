#ifndef RUNIR_KR_PS_ICP_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_ICP_COMPATIBILITY_HPP_

#include "runir/kr/ps/compatibility.hpp"
#include "runir/kr/ps/icp/rule_variant_view.hpp"
#include "runir/kr/ps/icp/rule_view.hpp"

#include <concepts>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::icp
{

using runir::kr::ps::is_compatible_with;

template<RuleKind Kind, typename C, typename Context>
bool conditions_are_compatible(ygg::View<ygg::Index<Rule<Kind>>, C> rule, Context& context)
{
    for (auto condition : rule.get_conditions())
        if (!runir::kr::ps::icp::is_compatible_with(condition, context))
            return false;

    return true;
}

template<RuleKind Kind, typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<Rule<Kind>>, C> rule, Context& context)
{
    if (!conditions_are_compatible(rule, context))
        return false;

    if constexpr (requires { rule.get_effects(); })
    {
        for (auto effect : rule.get_effects())
            if (!runir::kr::ps::icp::is_compatible_with(effect, context))
                return false;
    }

    return true;
}

template<typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<ps::Rule<IcpFamilyTag>>, C> rule, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::icp::is_compatible_with(child, context); }, rule.get_variant());
}

}  // namespace runir::kr::ps::icp

#endif
