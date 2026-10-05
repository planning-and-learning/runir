#ifndef RUNIR_KR_PS_EXT_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_EXT_COMPATIBILITY_HPP_

#include "runir/kr/ps/compatibility.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"

#include <concepts>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext
{

using runir::kr::ps::is_compatible_with;

template<tyr::TaskKind Kind, RuleKind Tag, typename C, typename Context>
bool conditions_are_compatible(ygg::View<ygg::Index<Rule<Tag>>, C> rule, Context& context)
{
    return all_compatible<Kind>(rule.get_conditions(), context);
}

template<RuleKind RuleKindT, typename C, typename S>
bool has_current_source(ygg::View<ygg::Index<Rule<RuleKindT>>, C> rule, S state)
{
    return rule.get_source() == state.get_module_state().get_memory_state();
}

// Reject the source memory before evaluating conditions in the reusable environment.
template<tyr::TaskKind Kind, RuleKind RuleKindT, typename C, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
bool rule_is_applicable(ygg::View<ygg::Index<Rule<RuleKindT>>, C> rule, S state, const PS& planning_state, EvaluationEnvironment<Kind>& environment)
{
    if (!has_current_source(rule, state))
        return false;
    auto state_context = environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
    return conditions_are_compatible<Kind>(rule, state_context);
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
bool is_compatible_with(ygg::View<ygg::Index<ps::Rule<ExtFamilyTag>>, C> rule, Context& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with<Kind>(child, context); }, rule.get_variant());
}

}  // namespace runir::kr::ps::ext

#endif
