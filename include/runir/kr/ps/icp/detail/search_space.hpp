#ifndef RUNIR_KR_PS_ICP_DETAIL_SEARCH_SPACE_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_SEARCH_SPACE_HPP_

#include "runir/kr/ps/icp/detail/search_node.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <cstddef>
#include <tyr/planning/plan.hpp>
#include <utility>
#include <vector>

namespace runir::kr::ps::icp::detail
{

template<tyr::TaskKind Kind>
tyr::planning::PackedPlan<Kind> extract_total_ordered_plan(std::size_t goal,
                                                           const std::vector<SearchNode<Kind>>& nodes,
                                                           const tyr::planning::PackedNode<Kind>& initial_node,
                                                           runir::kr::TaskContext<Kind>& context)
{
    auto actions = std::vector<tyr::formalism::planning::ActionBindingView> {};
    auto index = goal;
    while (nodes[index].parent)
    {
        if (nodes[index].action)
            actions.push_back(*nodes[index].action);
        index = *nodes[index].parent;
    }
    std::ranges::reverse(actions);
    // Reconstruct plan states and cumulative metrics from the recorded actions.
    auto steps = tyr::planning::PackedLabeledNodeList<Kind> {};
    steps.reserve(actions.size());
    auto node = initial_node.unpack();
    auto& search = *context.search_context;
    for (const auto action : actions)
    {
        node = search.successor_generator->get_successor_node(node, action, *search.state_repository, *search.axiom_evaluator);
        steps.push_back({ action, node.pack() });
    }
    return tyr::planning::PackedPlan<Kind>(initial_node, std::move(steps));
}

}  // namespace runir::kr::ps::icp::detail

#endif
