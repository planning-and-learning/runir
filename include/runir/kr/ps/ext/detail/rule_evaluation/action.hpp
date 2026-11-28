#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_ACTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_ACTION_HPP_

#include "runir/datasets/successors.hpp"
#include "runir/kr/dl/object_rows.hpp"
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
#include <tyr/formalism/repository.hpp>
#include <tyr/planning/nodes/labeled_node_view.hpp>
#include <tyr/planning/nodes/node_view.hpp>
#include <tyr/planning/states/state_repository.hpp>
#include <tyr/planning/successors/ground/successor_generator.hpp>
#include <tyr/planning/successors/lifted/successor_generator.hpp>
#include <tyr/planning/successors/successor_generator.hpp>
#include <tyr/planning/tasks/ground/task.hpp>
#include <tyr/planning/tasks/lifted/task.hpp>
#include <urd/config.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
class ActionRuleEvaluator
{
    RuleView<ActionTag> m_rule;
    RuleVariantView m_variant;
    tyr::formalism::ActionView<tyr::LiftedTag> m_action;
    // Retained scratch for decoding query rows; each use completes before any callback.
    /// The offered binding (the action and the row's objects), read through the planning repository when checked.
    /// The binding offered for the action, being filled: a row's builder.
    mutable urd::builder<ygg::Row<tyr::formalism::Action<tyr::LiftedTag>>, tyr::formalism::Repository> m_offer;
    mutable urd::Store<urd::sequence<ygg::Value>> m_row;

    static void require_applicable(tyr::planning::ActionBindingStatus status)
    {
        if (status == tyr::planning::ActionBindingStatus::OUTSIDE_PARAMETER_DOMAIN)
            throw std::logic_error("Action rule: object is outside the action parameter domain.");
        if (status != tyr::planning::ActionBindingStatus::APPLICABLE)
            throw std::logic_error("Action rule: offered action is not applicable.");
    }

public:
    using RuleTag = ActionTag;

    ActionRuleEvaluator(RuleView<ActionTag> rule, RuleVariantView variant, tyr::formalism::ActionView<tyr::LiftedTag> action) :
        m_rule(rule),
        m_variant(variant),
        m_action(action)
    { urd::store(m_offer.relation, action); }

    auto get_rule() const noexcept { return m_rule; }

    auto get_variant() const noexcept { return m_variant; }

private:
    template<runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             urd::ViewOf<ProgramState<Kind>> State,
             tyr::planning::StateBuilderViewConcept<Kind> PlanningState,
             tyr::planning::StateBuilderViewConcept<Kind> Candidate>
    bool effects_hold(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context,
                      State state,
                      const runir::datasets::PlanningNode<Kind, Candidate>& candidate) const
    {
        context.environment->reset_target();
        auto transition = urd::visit_view(state.module_state(),
                                          [&](const auto& activation)
                                          {
                                              return context.environment->make_dl_transition_context(context.planning_state,
                                                                                                     candidate.state(),
                                                                                                     activation.arguments(),
                                                                                                     activation.registers(),
                                                                                                     activation.registers());
                                          });
        return all_compatible<Kind, ExtFamilyTag>(m_rule.effects(), transition);
    }

public:
    template<runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             EmitConcept<ProgramStep<ExtFamilyTag, Kind, StorageStoredState<Storage>>> Emit,
             StopConcept Stop,
             ExecutionStateViewConcept<Storage> State,
             tyr::planning::StateBuilderViewConcept<Kind> PlanningState>
    bool
    emit(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context, State state, const Emit& emit, const Stop& stop) const
    {
        const auto& planning_state = context.planning_state;
        const auto rule = m_rule;
        const auto rule_variant = m_variant;
        if (!ext::rule_is_applicable(rule, state, planning_state, *context.environment))
            return true;
        auto state_context = context.make_dl_context(state);
        const auto query = evaluate<Kind>(rule.query_feature(), state_context);
        // Tyr reads a source from a node: one for every offer of this state. Policy applicability has no carried path metric.
        const auto planning_node = runir::datasets::node_of<Kind>(planning_state);
        auto& search = *(*context.task_context)->search_context;
        for (const auto row : query.rows())
        {
            if (stop())
                return false;
            m_offer.values.clear();
            ygg::with_values(row,
                             [this](const auto& values)
                             {
                                 for (const ygg::Value value : values)
                                     m_offer.values.push_back(value);
                             });
            const auto offered =
                runir::datasets::applicable_action_binding(search, planning_node, urd::resolve(m_offer, *planning_state.formalism_repository()));
            require_applicable(offered.status);
            // An applicable offer carries its binding.
            URD_ASSUME(offered.binding.has_value());
            const auto candidate = context.storage->successor(planning_node, *offered.binding);
            // Effects filter the offered transitions, as for Do rules.
            if (!effects_hold(context, state, candidate))
                continue;
            const auto labeled = runir::datasets::LabeledPlanningNode<Kind, typename Storage::PlanningState> { *offered.binding, candidate };
            if (!emit(detail::planning_step(*context.storage, state, labeled, rule_variant, rule.target(), *context.task_context)))
                return false;
        }
        return true;
    }

    template<runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             urd::ViewOf<ProgramState<Kind>> State,
             tyr::planning::StateBuilderViewConcept<Kind> PlanningState,
             tyr::planning::StateBuilderViewConcept<Kind> Candidate,
             ygg::RowViewConcept<tyr::formalism::Action<tyr::LiftedTag>> Binding>
    bool matches(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context,
                 State state,
                 const runir::datasets::LabeledPlanningNode<Kind, Candidate, Binding>& candidate) const
    {
        const auto& planning_state = context.planning_state;
        const auto rule = m_rule;
        const auto action = candidate.label().relation();
        if (!ext::rule_is_applicable(rule, state, planning_state, *context.environment) || !urd::is_interned_as(action, m_action))
            return false;
        auto state_context = context.make_dl_context(state);
        const auto query = evaluate<Kind>(rule.query_feature(), state_context);
        const auto& label = candidate.label();
        if (label.values().size() != query.columns().size()
            || !query.contains(runir::kr::dl::encode_object_row(label.values(), m_row)))  // object columns: canonical values
            return false;
        auto& generator = *(*context.task_context)->search_context->successor_generator;
        const auto node = runir::datasets::node_of<Kind>(planning_state);
        const auto status = generator.check_action_binding(node, candidate.label());
        require_applicable(status);
        return effects_hold(context, state, candidate.node());
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
