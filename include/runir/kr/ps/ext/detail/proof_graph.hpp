#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_GRAPH_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/ps/ext/detail/predecessors.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"

#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/containers/segmented_vector.hpp>

namespace runir::kr::ps::ext::detail
{

/// Materialize every recorded transition in discovery order, preserving parallel edges and rejected branches.
/// Open/deadend/cycle diagnostics describe the explored graph; they do not override the search's AND/OR result.
template<tyr::TaskKind Kind>
void build_proof_graph(ProgramProofResults<Kind>& result,
                       const ygg::SegmentedVector<SearchNode<Kind>>& nodes,
                       const Predecessors<Kind>& predecessors,
                       std::optional<ProgramStateView<Kind>> initial)
{
    using VertexLabel = ProgramProofVertexLabel<Kind>;
    using Edge = std::tuple<graphs::VertexIndex, graphs::VertexIndex, ProgramProofEdgeLabel>;
    constexpr auto no_vertex = std::numeric_limits<graphs::VertexIndex>::max();

    auto graph_indices = std::vector<graphs::VertexIndex>(nodes.size(), no_vertex);
    auto vertices = std::vector<VertexLabel> {};
    result.deadend_states.clear();
    result.open_states.clear();
    const auto add_vertex = [&](ProgramStateView<Kind> state)
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
        auto label = ProgramProofEdgeLabel {};
        if (predecessor.action)
            label.action = *predecessor.action;
        if (predecessor.rule)
            label.rule = *predecessor.rule;
        edges.emplace_back(source, target, std::move(label));
    }
    result.graph = std::make_shared<ProgramProofGraph<Kind>>(std::span<const VertexLabel>(vertices), std::span<const Edge>(edges));
    result.cycle = graphs::find_cycle(*result.graph);
}

}  // namespace runir::kr::ps::ext::detail

#endif
