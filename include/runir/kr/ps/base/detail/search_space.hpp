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
tyr::planning::PackedPlan<Kind> extract_total_ordered_plan(tyr::planning::PackedNode<Kind> goal,
                                                        const ygg::SegmentedVector<SearchNode<Kind>>& search_nodes)
{
    auto steps = tyr::planning::PackedLabeledNodeList<Kind> {};
    auto current = std::move(goal);
    while (true)
    {
        const auto& node = search_nodes[ygg::uint_t(current.get_state().get_index())];
        if (!node.parent_node)
            break;
        assert(node.action);
        steps.push_back({ *node.action, current });
        current = *node.parent_node;
    }
    std::ranges::reverse(steps);
    return tyr::planning::PackedPlan<Kind>(std::move(current), std::move(steps));
}

}  // namespace runir::kr::ps::base::detail

#endif
