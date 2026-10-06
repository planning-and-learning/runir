#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <tyr/planning/node.hpp>

namespace runir::kr::ps::ext::detail
{

struct DoRuleWorkspace;
struct ChooseRuleWorkspace;

}  // namespace runir::kr::ps::ext::detail

namespace runir::kr::ps
{

/// Borrows execution services and retains one prepared planning-state view for the evaluation.
/// Rule evaluators never own or refer back to an expander.
template<tyr::TaskKind Kind, ext::ExecutionStorageConcept<Kind> Storage, tyr::planning::StateViewConcept<Kind> PlanningState>
struct RuleEvaluationContext<runir::kr::ExtFamilyTag, Kind, Storage, PlanningState>
{
    using FamilyType = runir::kr::ExtFamilyTag;

    const runir::kr::TaskContextPtr<Kind>& task_context;
    Storage& storage;
    ext::EvaluationEnvironment<Kind>& environment;
    ext::detail::DoRuleWorkspace& do_workspace;
    ext::detail::ChooseRuleWorkspace& choose_workspace;
    const PlanningState planning_state;

    template<ext::ProgramStateViewConcept<Kind> State>
    auto make_dl_context(State state)
    {
        const auto module_ = state.get_module_state();
        return environment.make_dl_context(planning_state, module_.get_arguments(), module_.get_registers());
    }

    template<ext::ProgramStateViewConcept<Kind> State,
             ygg::formalism::RelationBindingViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>, tyr::formalism::ObjectTag> Binding>
    auto make_dl_transition_context(State state, const tyr::planning::LabeledNode<Kind, PlanningState, Binding>& candidate)
    {
        const auto module_ = state.get_module_state();
        return environment.make_dl_transition_context(planning_state,
                                                      candidate.node.get_state(),
                                                      module_.get_arguments(),
                                                      module_.get_registers(),
                                                      module_.get_registers());
    }
};

}  // namespace runir::kr::ps

#endif
