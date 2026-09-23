#ifndef RUNIR_KR_PS_BASE_DETAIL_SEARCH_SPACE_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_SEARCH_SPACE_HPP_

#include "runir/kr/ps/base/detail/search_node.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <cassert>
#include <tyr/planning/plan.hpp>
#include <utility>
#include <vector>

namespace runir::kr::ps::base::detail
{

template<tyr::TaskKind Kind>
tyr::planning::PackedPlan<Kind> extract_total_ordered_plan(tyr::planning::PackedStateView<Kind> goal,
                                                           const ygg::SegmentedVector<SearchNode<Kind>>& search_nodes,
                                                           const tyr::planning::PackedNode<Kind>& initial_node,
                                                           runir::kr::TaskContext<Kind>& context)
{
    auto actions = std::vector<tyr::formalism::planning::ActionBindingView> {};
    auto state = std::move(goal);
    while (true)
    {
        const auto& node = search_nodes[ygg::uint_t(state.get_index())];
        if (!node.parent_state)
            break;
        assert(node.action);
        actions.push_back(*node.action);
        state = *node.parent_state;
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

}  // namespace runir::kr::ps::base::detail

#endif
