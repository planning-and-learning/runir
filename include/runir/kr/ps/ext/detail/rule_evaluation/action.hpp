#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_ACTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_ACTION_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/state_view.hpp>
#include <tyr/planning/successor_generator.hpp>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
class ActionRuleEvaluator
{
    RuleView<ActionTag> m_rule;
    RuleVariantView m_variant;
    tyr::formalism::planning::ActionView<tyr::LiftedTag> m_action;
    // Retained scratch for decoding query rows; each use completes before any callback.
    mutable std::vector<ygg::Index<tyr::formalism::Object>> m_objects;
    mutable std::vector<std::byte> m_row;

    static void require_applicable(tyr::planning::ActionBindingStatus status)
    {
        if (status == tyr::planning::ActionBindingStatus::OUTSIDE_PARAMETER_DOMAIN)
            throw std::logic_error("Action rule: object is outside the action parameter domain.");
        if (status != tyr::planning::ActionBindingStatus::APPLICABLE)
            throw std::logic_error("Action rule: offered action is not applicable.");
    }

public:
    using RuleTag = ActionTag;
    ActionRuleEvaluator(RuleView<ActionTag> rule, RuleVariantView variant, tyr::formalism::planning::ActionView<tyr::LiftedTag> action) :
        m_rule(rule),
        m_variant(variant),
        m_action(action)
    {
    }
    auto get_rule() const noexcept { return m_rule; }
    auto get_variant() const noexcept { return m_variant; }

private:
    template<runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             ProgramStateViewConcept<Kind> State,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    void check_action_effects(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context,
                              State state,
                              const tyr::planning::Node<Kind, PlanningState>& candidate) const
    {
        context.environment.reset_target();
        auto transition = context.environment.make_dl_transition_context(context.planning_state,
                                                                         candidate.get_state(),
                                                                         state.get_module_state().get_arguments(),
                                                                         state.get_module_state().get_registers(),
                                                                         state.get_module_state().get_registers());
        if (!all_compatible<Kind>(m_rule.get_effects(), transition))
            throw std::logic_error("Action rule: offered transition violates declared effects.");
    }

public:
    template<runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             EmitConcept<ProgramStep<Kind, Storage>> Emit,
             StopConcept Stop,
             ExecutionStateViewConcept<Storage> State,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    bool emit(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context, State state, Emit&& emit, Stop&& stop) const
    {
        const auto& planning_state = context.planning_state;
        const auto rule = m_rule;
        const auto rule_variant = m_variant;
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment))
            return true;
        auto state_context = context.make_dl_context(state);
        const auto query = evaluate<Kind>(rule.get_query_feature(), state_context);
        for (const auto row : query)
        {
            if (stop())
                return false;
            // Policy applicability has no carried path metric.
            const auto offered = context.task_context->search_context->successor_generator->try_get_applicable_action_binding(
                tyr::planning::Node<Kind, PlanningState>(planning_state, 0),
                m_action,
                tyr::formalism::planning::ObjectSpanView(row.get_data().get_all(m_objects), *planning_state.get_formalism_repository()));
            require_applicable(offered.status);
            const auto candidate = context.storage.successor(planning_state, *offered.binding);
            check_action_effects(context, state, candidate);
            const auto labeled = tyr::planning::LabeledNode<Kind, PlanningState> { *offered.binding, candidate };
            if (!emit(detail::planning_step(context.storage, state, labeled, rule_variant, rule.get_target(), context.task_context)))
                return false;
        }
        return true;
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
        const auto& planning_state = context.planning_state;
        const auto rule = m_rule;
        const auto action = candidate.label.get_relation();
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment) || action != m_action)
            return false;
        auto state_context = context.make_dl_context(state);
        const auto query = evaluate<Kind>(rule.get_query_feature(), state_context);
        const auto objects = candidate.label.get_objects();
        if (objects.size() != query.arity() || !query.contains(ygg::database::encode_row<runir::kr::dl::QueryValues, ygg::Index<tyr::formalism::Object>>(objects.get_data(), m_row)))
            return false;
        auto& generator = *context.task_context->search_context->successor_generator;
        const auto node = tyr::planning::Node<Kind, PlanningState>(planning_state, 0);
        const auto status = generator.check_action_binding(node, candidate.label);
        require_applicable(status);
        check_action_effects(context, state, candidate.node);
        return true;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
