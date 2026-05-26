#ifndef RUNIR_KR_PS_EXT_DETAIL_PLAN_TRACE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PLAN_TRACE_HPP_

#include <optional>
#include <tyr/formalism/planning/ground_action_index.hpp>
#include <tyr/planning/algorithms/utils.hpp>
#include <tyr/planning/node.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
std::optional<tyr::Index<tyr::formalism::planning::GroundAction>> find_transition_action(const tyr::planning::StateView<Kind>& source_state,
                                                                                         const tyr::planning::StateView<Kind>& target_state,
                                                                                         const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    if (source_state.get_index() == target_state.get_index())
        return std::nullopt;

    for (const auto& successor : successors)
        if (successor.node.get_state().get_index() == target_state.get_index())
            return successor.label.get_index();

    return std::nullopt;
}

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

template<tyr::planning::TaskKind Kind>
std::optional<tyr::Index<tyr::formalism::planning::GroundAction>> first_plan_action(const tyr::planning::SearchResult<Kind>& search_result)
{
    if (!search_result.plan)
        return std::nullopt;

    const auto& suffix = search_result.plan->get_labeled_succ_nodes();
    if (suffix.empty())
        return std::nullopt;

    return suffix.front().label.get_index();
}

}  // namespace runir::kr::ps::ext::detail

#endif
