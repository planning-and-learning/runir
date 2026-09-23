#ifndef RUNIR_KR_PS_BASE_DETAIL_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_PROOF_GRAPH_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/ps/base/detail/search_node.hpp"
#include "runir/kr/ps/base/sketch_executor_data.hpp"

#include <limits>
#include <memory>
#include <optional>
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
                       std::optional<tyr::planning::PackedStateView<Kind>> initial)
{
    using VertexLabel = SketchProofVertexLabel<Kind>;
    using Edge = std::tuple<graphs::VertexIndex, graphs::VertexIndex, SketchProofEdgeLabel>;
    constexpr auto no_vertex = std::numeric_limits<graphs::VertexIndex>::max();

    auto graph_indices = std::vector<graphs::VertexIndex>(nodes.size(), no_vertex);
    auto vertices = std::vector<VertexLabel> {};
    result.deadend_states.clear();
    result.open_states.clear();
    const auto add_vertex = [&](const tyr::planning::PackedStateView<Kind>& state)
    {
        const auto index = ygg::uint_t(state.get_index());
        auto& vertex = graph_indices[index];
        if (vertex != no_vertex)
            return vertex;
        vertex = static_cast<graphs::VertexIndex>(vertices.size());
        const auto& node = nodes[index];
        if (node.is_deadend)
            result.deadend_states.push_back(vertex);
        if (node.is_open)
            result.open_states.push_back(vertex);
        vertices.emplace_back(state, initial == state, node.is_goal, !node.is_unsolvable, node.is_unsolvable);
        return vertex;
    };
    if (initial)
        add_vertex(*initial);

    auto edges = std::vector<Edge> {};
    edges.reserve(predecessors.size());
    for (const auto& predecessor : predecessors)
    {
        const auto source = add_vertex(predecessor.source);
        const auto target = add_vertex(predecessor.target);
        edges.emplace_back(source, target, SketchProofEdgeLabel(predecessor.action, predecessor.rule));
    }
    result.graph = std::make_shared<SketchProofGraph<Kind>>(std::span<const VertexLabel>(vertices), std::span<const Edge>(edges));
    result.cycle = graphs::find_cycle(*result.graph);
}

}  // namespace runir::kr::ps::base::detail

#endif
