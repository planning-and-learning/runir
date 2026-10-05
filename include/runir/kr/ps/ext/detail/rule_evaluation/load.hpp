#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_LOAD_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_LOAD_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/binding.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <concepts>
#include <utility>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind, runir::kr::dl::ConceptOrRoleTag Category>
class LoadRuleEvaluator
{
    RuleView<LoadTag<Category>> m_rule;
    RuleVariantView m_variant;

public:
    using RuleTag = LoadTag<Category>;
    LoadRuleEvaluator(RuleView<LoadTag<Category>> rule, RuleVariantView variant) : m_rule(rule), m_variant(variant) {}
    auto get_rule() const noexcept { return m_rule; }
    auto get_variant() const noexcept { return m_variant; }

    template<ExecutionStorageConcept<Kind> Storage,
             EmitConcept<ProgramStep<Kind, Storage>> Emit,
             StopConcept Stop,
             ExecutionStateViewConcept<Storage> State,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    bool emit(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context, State state, Emit&& emit, Stop&& stop) const
    {
        const auto& planning_state = context.planning_state;
        const auto rule = m_rule;
        const auto rule_variant = m_variant;
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment))
            return true;
        auto state_context = context.make_dl_context(state);
        const auto denotation = evaluate<Kind>(rule.get_feature(), state_context);
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(context.task_context->dl_builder);
        for (const auto value : denotation)
        {
            if (stop())
                return false;
            const auto target_registers = bound_registers(rule, state.get_module_state().get_registers(), value, *registers, context.storage);
            if (!binding_effects_match(rule, state, planning_state, target_registers, context.environment))
                continue;
            const auto module_ = state.get_module_state();
            auto target = context.storage.store(planning_state,
                                                module_.get_module(),
                                                rule.get_target(),
                                                target_registers,
                                                module_.get_arguments(),
                                                state.get_call_stack());
            if (!emit(detail::applied<Kind, Storage>(std::move(target), rule_variant, context.task_context)))
                return false;
        }
        return true;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
