#ifndef RUNIR_KR_PS_DETAIL_SEARCH_SPACE_HPP_
#define RUNIR_KR_PS_DETAIL_SEARCH_SPACE_HPP_

#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <runir/datasets/successors.hpp>
#include <tyr/planning/plans/plan_view.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::ps::detail
{

/// The plan to `goal`, replayed from the initial node: the actions of the nodes on goal's parent path, the root's
/// excluded. `parent(node)` is the node's parent, or nullptr at the root.
template<tyr::TaskKind Kind, typename Node, typename Parent>
tyr::planning::OwnedPlanRecordView<tyr::planning::DeterministicTag, Kind>
extract_total_ordered_plan(const Node& goal, Parent parent, const tyr::planning::OwnedNodeRecordView<Kind>& initial_node, runir::kr::TaskContext<Kind>& context)
{
    auto actions = urd::Store<urd::sequence<tyr::formalism::ActionRowView>> {};
    for (const Node* node = &goal; const Node* next = parent(*node); node = next)
        if (node->action)
            actions.push_back(*node->action);
    std::ranges::reverse(actions);
    return runir::datasets::replayed_plan<Kind>(*context.search_context, initial_node, actions);
}

}  // namespace runir::kr::ps::detail

#endif
