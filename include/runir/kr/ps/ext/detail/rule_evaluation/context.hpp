#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CONTEXT_HPP_

#include "runir/datasets/successors.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <tyr/planning/nodes/labeled_node_view.hpp>
#include <tyr/planning/nodes/node_view.hpp>

namespace runir::kr::ps::ext::detail
{

struct DoRuleWorkspace;
struct ChooseRuleWorkspace;

}  // namespace runir::kr::ps::ext::detail

namespace runir::kr::ps
{

/// Borrows execution services and retains one prepared planning-state view for the evaluation.
/// Rule evaluators never own or refer back to an expander.
template<tyr::TaskKind Kind,
         ext::ExecutionStorageConcept<Kind> Storage,
         tyr::planning::StateBuilderViewConcept<Kind> PlanningState,
         runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
struct RuleEvaluationContext<runir::kr::ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>
{
    using FamilyType = runir::kr::ExtFamilyTag;

    const runir::kr::TaskContextPtr<Kind>* task_context;
    Storage* storage;
    ext::EvaluationEnvironment<Kind, EvaluationPolicy>* environment;
    ext::detail::DoRuleWorkspace* do_workspace;
    ext::detail::ChooseRuleWorkspace* choose_workspace;
    PlanningState planning_state;

    template<urd::ViewOf<ext::ProgramState<Kind>> State>
    auto make_dl_context(State state)
    {
        environment->select_source(state);
        return urd::visit_view(state.module_state(),
                               [&](const auto& module_) { return environment->make_dl_context(planning_state, module_.arguments(), module_.registers()); });
    }

    /// From this context's planning state to a candidate's (its storage's successor: of another state type, where this
    /// one is registered and the storage builds its successors).
    template<urd::ViewOf<ext::ProgramState<Kind>> State,
             tyr::planning::StateBuilderViewConcept<Kind> Candidate,
             ygg::RowViewConcept<tyr::formalism::Action<tyr::LiftedTag>> Binding>
    auto make_dl_transition_context(State state, const runir::datasets::LabeledPlanningNode<Kind, Candidate, Binding>& candidate)
    {
        environment->select_source(state);
        return urd::visit_view(state.module_state(),
                               [&](const auto& module_)
                               {
                                   return environment->make_dl_transition_context(planning_state,
                                                                                  candidate.node().state(),
                                                                                  module_.arguments(),
                                                                                  module_.registers(),
                                                                                  module_.registers());
                               });
    }
};

}  // namespace runir::kr::ps

#endif
