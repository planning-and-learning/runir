#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_BACKTRACK_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_BACKTRACK_HPP_

#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <utility>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
class BacktrackRuleEvaluator
{
    RuleView<BacktrackTag> m_rule;
    RuleVariantView m_variant;

public:
    using RuleTag = BacktrackTag;
    BacktrackRuleEvaluator(RuleView<BacktrackTag> rule, RuleVariantView variant) : m_rule(rule), m_variant(variant) {}
    auto get_rule() const noexcept { return m_rule; }
    auto get_variant() const noexcept { return m_variant; }

    template<runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             EmitConcept<ProgramStep<Kind, Storage>> Emit,
             StopConcept Stop,
             ExecutionStateViewConcept<Storage> State,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    bool emit(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context, State state, Emit&& emit, Stop&& stop) const
    {
        if (!ext::rule_is_applicable(m_rule, state, context.planning_state, context.environment))
            return true;
        if (stop())
            return false;
        auto failure = detail::make_step<Kind, Storage>(ProgramOutcome::FAILURE, context.storage.retain(state), context.task_context);
        failure.rule = m_variant;
        return emit(std::move(failure));
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
