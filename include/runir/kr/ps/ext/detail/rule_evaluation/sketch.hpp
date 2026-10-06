#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_SKETCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_SKETCH_HPP_

#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <cassert>
#include <concepts>
#include <tyr/planning/node.hpp>
#include <utility>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
class SketchRuleEvaluator
{
    RuleView<SketchTag> m_rule;
    RuleVariantView m_variant;

public:
    using RuleTag = SketchTag;
    SketchRuleEvaluator(RuleView<SketchTag> rule, RuleVariantView variant) : m_rule(rule), m_variant(variant) {}
    auto get_rule() const noexcept { return m_rule; }
    auto get_variant() const noexcept { return m_variant; }

    /// Emit a control-only rule; effectful rules must use matches() on the shared successor batch.
    template<runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             EmitConcept<ProgramStep<Kind, Storage>> Emit,
             StopConcept Stop,
             ExecutionStateViewConcept<Storage> State,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    bool emit(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context, State state, Emit&& emit, Stop&& stop) const
    {
        assert(m_rule.get_effects().empty() && "Effectful Sketch rules require the shared successor batch.");
        if (!ext::rule_is_applicable(m_rule, state, context.planning_state, context.environment))
            return true;
        if (stop())
            return false;
        const auto module_ = state.get_module_state();
        auto target = context.storage.store(context.planning_state,
                                            module_.get_module(),
                                            m_rule.get_target(),
                                            module_.get_registers(),
                                            module_.get_arguments(),
                                            state.get_call_stack());
        return emit(detail::applied<Kind, Storage>(std::move(target), m_variant, context.task_context));
    }

    template<runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             ProgramStateViewConcept<Kind> State,
             tyr::planning::StateViewConcept<Kind> PlanningState,
             ygg::formalism::RelationBindingViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>, tyr::formalism::ObjectTag> Binding>
    bool matches(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context,
                 State state,
                 const tyr::planning::LabeledNode<Kind, PlanningState, Binding>& candidate) const
    {
        if (m_rule.get_effects().empty() || !ext::has_current_source(m_rule, state))
            return false;
        auto transition = context.make_dl_transition_context(state, candidate);
        return is_compatible_with<Kind>(m_rule, transition);
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
