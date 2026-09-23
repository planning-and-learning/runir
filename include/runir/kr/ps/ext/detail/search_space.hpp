#ifndef RUNIR_KR_PS_EXT_DETAIL_SEARCH_SPACE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SEARCH_SPACE_HPP_

#include "runir/kr/ps/ext/detail/search_node.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"

#include <algorithm>
#include <tyr/planning/plan.hpp>
#include <utility>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
tyr::planning::PackedPlan<Kind> extract_total_ordered_plan(ygg::Index<ProgramState<Kind>> goal,
                                                        const ygg::SegmentedVector<SearchNode<Kind>>& search_nodes,
                                                        const ExecutionRepository<Kind>& execution_repository,
                                                        const tyr::planning::PackedNode<Kind>& initial_node)
{
    auto steps = tyr::planning::PackedLabeledNodeList<Kind> {};
    auto state = goal;
    while (search_nodes[ygg::uint_t(state)].parent_state != ygg::Index<ProgramState<Kind>>::max())
    {
        const auto& node = search_nodes[ygg::uint_t(state)];
        if (node.action)
        {
            const auto module_state = ProgramStateView<Kind>(state, execution_repository).get_module_state();
            const auto packed_state = tyr::planning::PackedStateView<Kind>(module_state.get_data().state, initial_node.get_state().get_state_repository());
            steps.push_back({ *node.action, tyr::planning::PackedNode<Kind>(packed_state, node.metric) });
        }
        state = node.parent_state;
    }
    std::ranges::reverse(steps);
    return tyr::planning::PackedPlan<Kind>(initial_node, std::move(steps));
}

}  // namespace runir::kr::ps::ext::detail

#endif
