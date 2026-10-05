#ifndef RUNIR_KR_PS_EXT_DETAIL_SEARCH_SPACE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SEARCH_SPACE_HPP_

#include "runir/kr/ps/ext/detail/search_node.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <span>
#include <tyr/planning/plan.hpp>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
tyr::planning::PackedPlan<Kind> extract_total_ordered_plan(std::span<const tyr::formalism::planning::ActionBindingView> actions,
                                                           const tyr::planning::PackedNode<Kind>& initial_node,
                                                           runir::kr::TaskContext<Kind>& context)
{
    auto& search = *context.search_context;
    return tyr::planning::replay_plan<Kind>(initial_node, actions, *search.successor_generator, *search.state_repository, *search.axiom_evaluator);
}

template<tyr::TaskKind Kind>
tyr::planning::PackedPlan<Kind> extract_total_ordered_plan(ProgramStateView<Kind> goal,
                                                           const ygg::SegmentedVector<SearchNode<Kind>>& search_nodes,
                                                           const tyr::planning::PackedNode<Kind>& initial_node,
                                                           runir::kr::TaskContext<Kind>& context)
{
    auto actions = std::vector<tyr::formalism::planning::ActionBindingView> {};
    auto state = goal;
    while (search_nodes[ygg::uint_t(state.get_index())].parent_state)
    {
        const auto& node = search_nodes[ygg::uint_t(state.get_index())];
        if (node.action)
            actions.push_back(*node.action);
        state = *node.parent_state;
    }
    std::ranges::reverse(actions);
    // Reconstruct plan states and cumulative metrics from the recorded actions.
    return extract_total_ordered_plan<Kind>(actions, initial_node, context);
}

}  // namespace runir::kr::ps::ext::detail

#endif
