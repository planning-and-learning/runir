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
    auto& search = *context.search_context;
    return tyr::planning::replay_plan<Kind>(initial_node, actions, *search.successor_generator, *search.state_repository, *search.axiom_evaluator);
}

}  // namespace runir::kr::ps::icp::detail

#endif
