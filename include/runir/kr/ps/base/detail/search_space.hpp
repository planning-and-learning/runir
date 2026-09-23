#ifndef RUNIR_KR_PS_BASE_DETAIL_SEARCH_SPACE_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_SEARCH_SPACE_HPP_

#include "runir/kr/ps/base/detail/search_node.hpp"

#include <algorithm>
#include <cassert>
#include <tyr/planning/plan.hpp>
#include <utility>

namespace runir::kr::ps::base::detail
{

template<tyr::TaskKind Kind>
tyr::planning::PackedPlan<Kind> extract_total_ordered_plan(ygg::Index<tyr::planning::State<Kind>> goal,
                                                        const ygg::SegmentedVector<SearchNode<Kind>>& search_nodes,
                                                        const tyr::planning::PackedNode<Kind>& initial_node)
{
    auto steps = tyr::planning::PackedLabeledNodeList<Kind> {};
    auto state = goal;
    while (search_nodes[ygg::uint_t(state)].parent_state != ygg::Index<tyr::planning::State<Kind>>::max())
    {
        const auto& node = search_nodes[ygg::uint_t(state)];
        assert(node.action);
        const auto packed_state = tyr::planning::PackedStateView<Kind>(state, initial_node.get_state().get_state_repository());
        steps.push_back({ *node.action, tyr::planning::PackedNode<Kind>(packed_state, node.metric) });
        state = node.parent_state;
    }
    std::ranges::reverse(steps);
    return tyr::planning::PackedPlan<Kind>(initial_node, std::move(steps));
}

}  // namespace runir::kr::ps::base::detail

#endif
