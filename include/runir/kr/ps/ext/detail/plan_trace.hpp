#ifndef RUNIR_KR_PS_EXT_DETAIL_PLAN_TRACE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PLAN_TRACE_HPP_

#include <tyr/planning/algorithms/utils.hpp>
#include <tyr/planning/node.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
void append_plan_suffix(tyr::planning::LabeledNodeList<Kind>& target, const tyr::planning::SearchResult<Kind>& search_result)
{
    if (!search_result.plan)
        return;

    const auto& suffix = search_result.plan->get_labeled_succ_nodes();
    target.insert(target.end(), suffix.begin(), suffix.end());
}

template<tyr::planning::TaskKind Kind>
void append_single_step_plan(tyr::planning::LabeledNodeList<Kind>& target,
                             const tyr::planning::StateView<Kind>& source_state,
                             const tyr::planning::StateView<Kind>& target_state,
                             const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    if (source_state.get_index() == target_state.get_index())
        return;

    for (const auto& successor : successors)
    {
        if (successor.node.get_state().get_index() == target_state.get_index())
        {
            target.push_back(successor);
            return;
        }
    }
}


}  // namespace runir::kr::ps::ext::detail

#endif
