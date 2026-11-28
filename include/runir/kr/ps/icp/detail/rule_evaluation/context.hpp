#ifndef RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_CONTEXT_HPP_

#include "runir/datasets/successors.hpp"
#include "runir/kr/ps/icp/detail/rule_evaluation/workspace.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <tyr/planning/nodes/labeled_node_view.hpp>
#include <tyr/planning/nodes/node_view.hpp>

namespace runir::kr::ps
{

/// Borrows shared scratch and retains the source planning view for one expansion.
template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
struct RuleEvaluationContext<IcpFamilyTag, Kind, void, tyr::planning::SharedStateBuilderView<Kind>, EvaluationPolicy>
{
    using FamilyType = IcpFamilyTag;

    icp::detail::RuleEvaluationWorkspace<Kind, EvaluationPolicy>* workspace;
    tyr::planning::SharedStateBuilderView<Kind> planning_state;

    auto make_dl_context(icp::ProgramStateView<Kind> source) { return workspace->get_environment().make_dl_context(planning_state, source.registers()); }

    template<ygg::RowViewConcept<tyr::formalism::Action<tyr::LiftedTag>> Binding>
    auto make_dl_transition_context(icp::ProgramStateView<Kind> source,
                                    const runir::datasets::LabeledPlanningNode<Kind, tyr::planning::SharedStateBuilderView<Kind>, Binding>& candidate)
    { return workspace->get_environment().make_dl_transition_context(planning_state, candidate.node().state(), source.registers(), source.registers()); }

    /// Loading registers leaves the planning state unchanged.
    auto make_dl_transition_context(icp::ProgramStateView<Kind> source, runir::kr::dl::semantics::RegisterValuesView target_registers)
    { return workspace->get_environment().make_dl_transition_context(planning_state, planning_state, source.registers(), target_registers); }
};

}  // namespace runir::kr::ps

#endif
