#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_DO_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_DO_HPP_

#include "runir/datasets/successors.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <algorithm>
#include <concepts>
#include <functional>
#include <tyr/formalism/planning/action_view.hpp>
#include <tyr/planning/nodes/labeled_node_view.hpp>
#include <tyr/planning/nodes/node_view.hpp>
#include <tyr/planning/successors/ground/successor_generator.hpp>
#include <tyr/planning/successors/lifted/successor_generator.hpp>
#include <urd/bits.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext::detail
{

struct DoRuleWorkspace
{
    urd::Store<urd::sequence<urd::bitset_span<const ygg::uint_t>>> denotations;
};

template<tyr::TaskKind Kind>
class DoRuleEvaluator
{
    RuleView<DoTag> m_rule;
    RuleVariantView m_variant;
    tyr::formalism::ActionView<tyr::LiftedTag> m_action;

public:
    using RuleTag = DoTag;

    DoRuleEvaluator(RuleView<DoTag> rule, RuleVariantView variant, tyr::formalism::ActionView<tyr::LiftedTag> action) :
        m_rule(rule),
        m_variant(variant),
        m_action(action)
    {
    }

    [[nodiscard]] auto get_rule() const noexcept { return m_rule; }

    [[nodiscard]] auto get_variant() const noexcept { return m_variant; }

private:
    template<runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             urd::ViewOf<ProgramState<Kind>> State,
             tyr::planning::StateBuilderViewConcept<Kind> PlanningState>
    auto& evaluate_do_arguments(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context,
                                DoRuleWorkspace& workspace,
                                RuleView<DoTag> rule,
                                State state) const
    {
        const auto arguments = rule.arguments();
        auto& denotations = workspace.denotations;
        denotations.clear();
        auto state_context = context.make_dl_context(state);
        for (auto argument : arguments)
            denotations.push_back(evaluate<Kind>(argument, state_context).value());
        return denotations;
    }

    template<ygg::RowViewConcept<tyr::formalism::Action<tyr::LiftedTag>> Binding, typename ConceptDenotations>
    static bool action_matches_do_arguments(RuleView<DoTag> rule, Binding action, const ConceptDenotations& denotations)
    {
        if (urd::visit_view(action.relation(), [](const auto& relation) { return relation.name(); }) != rule.action_name())
            return false;
        if (action.values().size() != denotations.size())
            return false;
        // The canonical values of its object columns are the objects' indices (a lifted binding may be borrowed, its pool not
        // yet made, so it is not read typed).
        for (size_t i = 0; i < denotations.size(); ++i)
            if (!denotations[i].test(static_cast<ygg::uint_t>(action.value(i))))
                return false;
        return true;
    }

    template<runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             urd::ViewOf<ProgramState<Kind>> State,
             tyr::planning::StateBuilderViewConcept<Kind> PlanningState,
             tyr::planning::StateBuilderViewConcept<Kind> Candidate>
    bool do_effects_match(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context,
                          RuleView<DoTag> rule,
                          State state,
                          const PlanningState& planning_state,
                          const Candidate& target_state) const
    {
        auto transition = urd::visit_view(state.module_state(),
                                          [&](const auto& activation)
                                          {
                                              return context.environment->make_dl_transition_context(planning_state,
                                                                                                     target_state,
                                                                                                     activation.arguments(),
                                                                                                     activation.registers(),
                                                                                                     activation.registers());
                                          });
        return is_compatible_with<Kind>(rule, transition);
    }

    template<runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy,
             ExecutionStorageConcept<Kind> Storage,
             urd::ViewOf<ProgramState<Kind>> State,
             tyr::planning::StateBuilderViewConcept<Kind> PlanningState,
             tyr::planning::StateBuilderViewConcept<Kind> Candidate,
             ygg::RowViewConcept<tyr::formalism::Action<tyr::LiftedTag>> Binding>
    bool do_rule_matches(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context,
                         DoRuleWorkspace& workspace,
                         RuleView<DoTag> rule,
                         State state,
                         const PlanningState& planning_state,
                         Binding action,
                         const Candidate& target_state) const
    {
        if (!ext::rule_is_applicable(rule, state, planning_state, *context.environment))
            return false;
        const auto& denotations = evaluate_do_arguments(context, workspace, rule, state);
        return action_matches_do_arguments(rule, action, denotations) && do_effects_match(context, rule, state, planning_state, target_state);
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
        auto& workspace = *context.do_workspace;
        const auto rule = m_rule;
        const auto rule_variant = m_variant;
        if (!ext::rule_is_applicable(rule, state, planning_state, *context.environment))
            return true;
        const auto& denotations = evaluate_do_arguments(context, workspace, rule, state);
        if (std::ranges::any_of(denotations, [](const auto& denotation) { return denotation.count() == 0; }))
            return true;
        auto& search = *(*context.task_context)->search_context;
        // Tyr reads a source from a node: one for every binding of this state.
        const auto planning_node = runir::datasets::node_of<Kind>(planning_state);
        const auto accept = [&](tyr::planning::ActionBindingView<Kind> binding) { return action_matches_do_arguments(rule, binding, denotations); };
        const auto visit = [&](tyr::planning::ActionBindingView<Kind> binding)
        {
            if (stop())
                return false;
            const auto candidate = context.storage->successor(planning_node, binding);
            context.environment->reset_target();
            if (!do_effects_match(context, rule, state, planning_state, candidate.state()))
                return true;
            if (stop())
                return false;
            const auto labeled =
                runir::datasets::LabeledPlanningNode<Kind, typename Storage::PlanningState> { runir::datasets::interned_binding(search, binding), candidate };
            return emit(detail::planning_step(*context.storage, state, labeled, rule_variant, rule.target(), *context.task_context));
        };
        auto filter = tyr::planning::ActionBindingFilter<Kind> {};
        filter.action = m_action;
        // By reference: a std::function holds a reference inline, and so allocates nothing.
        filter.accept = std::ref(accept);
        filter.stop = std::ref(stop);
        return search.successor_generator->for_each_applicable_action_binding(planning_node, filter, std::ref(visit));
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
        return do_rule_matches(context, *context.do_workspace, m_rule, state, planning_state, candidate.label(), candidate.node().state());
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
