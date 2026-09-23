#ifndef RUNIR_KR_PS_BASE_DETAIL_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_PROOF_GRAPH_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/ps/base/detail/search_node.hpp"
#include "runir/kr/ps/base/sketch_executor_data.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <span>
#include <tuple>
#include <vector>
#include <yggdrasil/containers/segmented_vector.hpp>
#include <yggdrasil/containers/unordered_multi_map.hpp>
#include <yggdrasil/containers/unordered_set.hpp>

namespace runir::kr::ps::base::detail
{

template<tyr::TaskKind Kind>
void build_proof_graph(SketchProofResults<Kind>& result,
                       const ygg::SegmentedVector<SearchNode<Kind>>& nodes,
                       const ygg::UnorderedSet<Predecessor<Kind>>& predecessors,
                       StateIndex<Kind> initial,
                       std::span<const StateIndex<Kind>> deadends,
                       std::span<const StateIndex<Kind>> open)
{
    using VertexLabel = SketchProofVertexLabel<Kind>;
    using Edge = std::tuple<graphs::VertexIndex, graphs::VertexIndex, SketchProofEdgeLabel>;
    constexpr auto no_vertex = std::numeric_limits<graphs::VertexIndex>::max();
    constexpr auto unreached = SearchNode<Kind>::unreached;

    auto incoming = ygg::UnorderedMultiMap<StateIndex<Kind>, const Predecessor<Kind>*> {};
    incoming.reserve(predecessors.size());
    for (const auto& predecessor : predecessors)
        incoming.insert(predecessor.target, &predecessor);

    auto graph_indices = std::vector<graphs::VertexIndex>(nodes.size(), no_vertex);
    auto states = std::vector<StateIndex<Kind>> {};
    auto records = std::vector<const Predecessor<Kind>*> {};
    const auto enqueue = [&](StateIndex<Kind> state)
    {
        auto& vertex = graph_indices[ygg::uint_t(state)];
        if (vertex == no_vertex)
        {
            vertex = 0;
            states.push_back(state);
        }
    };
    // Leaves seed finished and partial paths; boundary nodes also seed closed cycles.
    for (std::size_t i = 0; i < nodes.size(); ++i)
        if (nodes[i].discovery_order != unreached && (!nodes[i].has_successor || nodes[i].boundary))
            enqueue(StateIndex<Kind>(static_cast<ygg::uint_t>(i)));
    for (std::size_t i = 0; i < states.size(); ++i)
        for (const auto* predecessor : incoming.values(states[i]))
        {
            records.push_back(predecessor);
            enqueue(predecessor->source);
        }

    std::ranges::sort(states, {}, [&](auto state) { return nodes[ygg::uint_t(state)].discovery_order; });
    std::ranges::sort(records, {}, [](const auto* predecessor) { return predecessor->order; });
    auto vertices = std::vector<VertexLabel> {};
    vertices.reserve(states.size());
    for (const auto state : states)
    {
        graph_indices[ygg::uint_t(state)] = static_cast<graphs::VertexIndex>(vertices.size());
        const auto& node = nodes[ygg::uint_t(state)];
        vertices.emplace_back(tyr::planning::PackedStateView<Kind>(state, result.task_context_owner->search_context->state_repository),
                              initial == state, node.is_goal, !node.is_unsolvable, node.is_unsolvable);
    }
    auto edges = std::vector<Edge> {};
    edges.reserve(records.size());
    for (const auto* predecessor : records)
        edges.emplace_back(graph_indices[ygg::uint_t(predecessor->source)], graph_indices[ygg::uint_t(predecessor->target)],
                           SketchProofEdgeLabel(datasets::StateGraphEdgeLabel(predecessor->action, predecessor->cost), predecessor->rule));
    result.graph = std::make_shared<SketchProofGraph<Kind>>(std::span<const VertexLabel>(vertices), std::span<const Edge>(edges));
    const auto map_diagnostics = [&](std::span<const StateIndex<Kind>> source, graphs::VertexIndexList& target)
    {
        target.clear();
        target.reserve(source.size());
        for (const auto state : source)
        {
            const auto vertex = graph_indices[ygg::uint_t(state)];
            assert(vertex != no_vertex);
            target.push_back(vertex);
        }
    };
    map_diagnostics(deadends, result.deadend_states);
    map_diagnostics(open, result.open_states);
    result.cycle = graphs::find_cycle(*result.graph);
}

}  // namespace runir::kr::ps::base::detail

#endif
