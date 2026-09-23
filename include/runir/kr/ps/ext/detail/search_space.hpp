#ifndef RUNIR_KR_PS_EXT_DETAIL_SEARCH_SPACE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SEARCH_SPACE_HPP_

#include "runir/kr/ps/ext/detail/search_node.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <tyr/planning/plan.hpp>
#include <utility>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
tyr::planning::PackedPlan<Kind> extract_total_ordered_plan(ygg::Index<ProgramState<Kind>> goal,
                                                           const ygg::SegmentedVector<SearchNode<Kind>>& search_nodes,
                                                           const tyr::planning::PackedNode<Kind>& initial_node,
                                                           runir::kr::TaskContext<Kind>& context)
{
    auto steps = tyr::planning::PackedLabeledNodeList<Kind> {};
    auto state = goal;
    while (search_nodes[ygg::uint_t(state)].parent_state != ygg::Index<ProgramState<Kind>>::max())
    {
        const auto& node = search_nodes[ygg::uint_t(state)];
        if (node.planning_successor)
            steps.push_back(*node.planning_successor);
        state = node.parent_state;
    }
    std::ranges::reverse(steps);
    // Search does not accumulate metrics. Reconstruct them along the actual plan.
    auto node = initial_node.unpack();
    auto& search = *context.search_context;
    for (auto& step : steps)
    {
        node = search.successor_generator->get_successor_node(node, step.label, *search.state_repository, *search.axiom_evaluator);
        step.node = node.pack();
    }
    return tyr::planning::PackedPlan<Kind>(initial_node, std::move(steps));
}

}  // namespace runir::kr::ps::ext::detail

#endif
