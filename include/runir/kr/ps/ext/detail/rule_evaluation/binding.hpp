#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_BINDING_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_BINDING_HPP_

#include "runir/kr/dl/semantics/interning.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"

namespace runir::kr::ps::ext::detail
{

/// Load and Choose bind one register using caller-owned scratch.
template<BindingRuleKind Tag, runir::kr::dl::semantics::RegisterValuesViewConcept R, typename Value, typename Storage>
auto bound_registers(RuleView<Tag> rule, R source, const Value& value, ygg::Data<runir::kr::dl::semantics::RegisterValues>& registers, Storage& storage)
{
    runir::kr::dl::semantics::assign(registers, source);
    runir::kr::dl::semantics::assign_register(registers, rule.get_register().get_identifier(), value);
    return storage.registers(registers);
}

template<tyr::TaskKind Kind,
         BindingRuleKind Tag,
         ProgramStateViewConcept<Kind> S,
         tyr::planning::StateViewConcept<Kind> PS,
         runir::kr::dl::semantics::RegisterValuesViewConcept R>
bool binding_effects_match(RuleView<Tag> rule, S state, const PS& planning_state, R registers, EvaluationEnvironment<Kind>& environment)
{
    if (rule.get_effects().empty())
        return true;
    environment.reset_target();
    auto transition = environment.make_dl_transition_context(planning_state,
                                                             planning_state,
                                                             state.get_module_state().get_arguments(),
                                                             state.get_module_state().get_registers(),
                                                             registers);
    return is_compatible_with(rule, transition);
}

}  // namespace runir::kr::ps::ext::detail

#endif
