#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_BINDING_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_BINDING_HPP_

#include "runir/kr/dl/semantics/interning.hpp"
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"

namespace runir::kr::ps::ext::detail
{

/// Load and Choose bind one register using caller-owned scratch.
template<tyr::TaskKind Kind, BindingRuleKind Tag, urd::ReadsAs<runir::kr::dl::semantics::RegisterValues> R, ExecutionStorageConcept<Kind> Storage>
auto bound_registers(RuleView<Tag> rule,
                     const R& source,
                     const runir::kr::dl::semantics::DenotationElementView<typename Tag::Category>& value,
                     urd::data<runir::kr::dl::semantics::RegisterValues>& registers,
                     Storage& storage)
{
    urd::visit_view(source, [&registers](const auto& values) { runir::kr::dl::semantics::assign(registers, values); });
    runir::kr::dl::semantics::assign_register(registers, rule.slot().identifier(), value);
    return storage.registers(registers);
}

template<tyr::TaskKind Kind,
         BindingRuleKind Tag,
         urd::ViewOf<ProgramState<Kind>> S,
         tyr::planning::StateBuilderViewConcept<Kind> PS,
         urd::ReadsAs<runir::kr::dl::semantics::RegisterValues> R,
         runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
bool binding_effects_match(RuleView<Tag> rule,
                           S state,
                           const PS& planning_state,
                           const R& registers,
                           EvaluationEnvironment<Kind, EvaluationPolicy>& environment)
{
    if (rule.effects().empty())
        return true;
    environment.reset_target();
    return urd::visit_view(state.module_state(),
                           [&](const auto& module_)
                           {
                               auto transition =
                                   environment.make_dl_transition_context(planning_state, planning_state, module_.arguments(), module_.registers(), registers);
                               return is_compatible_with<Kind>(rule, transition);
                           });
}

}  // namespace runir::kr::ps::ext::detail

#endif
