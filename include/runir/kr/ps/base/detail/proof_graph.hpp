#ifndef RUNIR_KR_PS_BASE_DETAIL_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_PROOF_GRAPH_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/ps/base/detail/search_node.hpp"
#include "runir/kr/ps/base/sketch_executor_data.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <span>
#include <tuple>
#include <vector>
#include <yggdrasil/containers/segmented_vector.hpp>

namespace runir::kr::ps::base::detail
{

template<tyr::TaskKind Kind>
void build_proof_graph(SketchProofResults<Kind>& result,
                       const ygg::SegmentedVector<SearchNode<Kind>>& nodes,
                       std::span<const Predecessor<Kind>> predecessors,
                       ygg::Index<tyr::planning::State<Kind>> initial)
{
    using VertexLabel = SketchProofVertexLabel<Kind>;
    using Edge = std::tuple<graphs::VertexIndex, graphs::VertexIndex, SketchProofEdgeLabel>;
    constexpr auto no_vertex = std::numeric_limits<graphs::VertexIndex>::max();
    constexpr auto unreached = SearchNode<Kind>::unreached;

    auto graph_indices = std::vector<graphs::VertexIndex>(nodes.size(), no_vertex);
    auto states = std::vector<ygg::Index<tyr::planning::State<Kind>>> {};
    for (std::size_t i = 0; i < nodes.size(); ++i)
        if (nodes[i].step != unreached)
            states.emplace_back(static_cast<ygg::uint_t>(i));
    std::ranges::sort(states, {}, [&](auto state) { return nodes[ygg::uint_t(state)].step; });

    auto vertices = std::vector<VertexLabel> {};
    vertices.reserve(states.size());
    result.deadend_states.clear();
    result.open_states.clear();
    for (const auto state : states)
    {
        const auto vertex = static_cast<graphs::VertexIndex>(vertices.size());
        graph_indices[ygg::uint_t(state)] = vertex;
        const auto& node = nodes[ygg::uint_t(state)];
        vertices.emplace_back(tyr::planning::PackedStateView<Kind>(state, result.task_context_owner->search_context->state_repository),
                              initial == state, node.is_goal, !node.is_unsolvable, node.is_unsolvable);
        if (node.is_deadend)
            result.deadend_states.push_back(vertex);
        if (node.is_open)
            result.open_states.push_back(vertex);
    }

    auto edges = std::vector<Edge> {};
    edges.reserve(predecessors.size());
    for (const auto& predecessor : predecessors)
        edges.emplace_back(graph_indices[ygg::uint_t(predecessor.source)], graph_indices[ygg::uint_t(predecessor.target)],
                           SketchProofEdgeLabel(datasets::StateGraphEdgeLabel(predecessor.action, predecessor.cost), predecessor.rule));
    result.graph = std::make_shared<SketchProofGraph<Kind>>(std::span<const VertexLabel>(vertices), std::span<const Edge>(edges));
    result.cycle = graphs::find_cycle(*result.graph);
}

}  // namespace runir::kr::ps::base::detail

#endif
