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

template<tyr::TaskKind Kind, RuleKind Tag, typename C, typename Context>
bool conditions_are_compatible(ygg::View<ygg::Index<Rule<Tag>>, C> rule, Context& context)
{
    return all_compatible<Kind>(rule.get_conditions(), context);
}

template<tyr::TaskKind Kind, RuleKind Tag, typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<Rule<Tag>>, C> rule, Context& context)
{
    if (!conditions_are_compatible<Kind>(rule, context))
        return false;

    if constexpr (requires { rule.get_effects(); })
        return all_compatible<Kind>(rule.get_effects(), context);

    return true;
}

template<tyr::TaskKind Kind, typename C, typename Context>
bool is_compatible_with(ygg::View<ygg::Index<ps::Rule<IcpFamilyTag>>, C> rule, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::icp::is_compatible_with<Kind>(child, context); }, rule.get_variant());
}

}  // namespace runir::kr::ps::icp

#endif
