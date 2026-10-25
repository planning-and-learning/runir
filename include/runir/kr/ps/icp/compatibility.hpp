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

template<tyr::TaskKind Kind, RuleKind Tag, typename C, CompatibilityContext<IcpFamilyTag, Kind> Context>
bool conditions_are_compatible(ygg::View<ygg::Index<Rule<Tag>>, C> rule, Context& context)
{ return all_compatible<Kind, IcpFamilyTag>(rule.get_conditions(), context); }

}  // namespace runir::kr::ps::icp

#endif
