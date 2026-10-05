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
    auto& search = *context.search_context;
    return tyr::planning::replay_plan<Kind>(initial_node, actions, *search.successor_generator, *search.state_repository, *search.axiom_evaluator);
}

}  // namespace runir::kr::ps::base::detail

#endif
