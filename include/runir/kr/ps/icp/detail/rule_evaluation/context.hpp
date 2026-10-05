#ifndef RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/ps/icp/detail/rule_evaluation/workspace.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <tyr/planning/node.hpp>

namespace runir::kr::ps
{

/// Borrows shared scratch and retains the source planning view for one expansion.
template<tyr::TaskKind Kind>
struct RuleEvaluationContext<IcpFamilyTag, Kind>
{
    using FamilyType = IcpFamilyTag;
    using KindType = Kind;

    icp::detail::RuleEvaluationWorkspace<Kind>& workspace;
    const tyr::planning::StateView<Kind> planning_state;

    auto make_dl_context(icp::ProgramStateView<Kind> source) { return workspace.get_environment().make_dl_context(planning_state, source.get_registers()); }

    template<ygg::formalism::RelationBindingViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>, tyr::formalism::ObjectTag> Binding>
    auto make_dl_transition_context(icp::ProgramStateView<Kind> source, const tyr::planning::LabeledNode<Kind, tyr::planning::StateView<Kind>, Binding>& candidate)
    {
        return workspace.get_environment().make_dl_transition_context(planning_state,
                                                                      candidate.node.get_state(),
                                                                      source.get_registers(),
                                                                      source.get_registers());
    }

    /// Loading registers leaves the planning state unchanged.
    auto make_dl_transition_context(icp::ProgramStateView<Kind> source, runir::kr::dl::semantics::RegisterValuesView target_registers)
    {
        return workspace.get_environment().make_dl_transition_context(planning_state, planning_state, source.get_registers(), target_registers);
    }
};

}  // namespace runir::kr::ps

#endif
