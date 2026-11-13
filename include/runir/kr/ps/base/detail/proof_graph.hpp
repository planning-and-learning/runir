#ifndef RUNIR_KR_PS_BASE_DETAIL_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_PROOF_GRAPH_HPP_

#include "runir/kr/ps/base/detail/search_node.hpp"
#include "runir/kr/ps/base/sketch_executor_data.hpp"

#include <memory>
#include <optional>
#include <span>
#include <tuple>
#include <urd/repr/maps/index_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <vefr/algorithms.hpp>

namespace runir::kr::ps::base::detail
{

template<tyr::TaskKind Kind>
void build_proof_graph(SketchProofResults<Kind>& result,
                       const SearchNodes<Kind>& nodes,
                       std::span<const Predecessor<Kind>> predecessors,
                       std::optional<tyr::planning::PackedStateView<Kind>> initial)
{
    using VertexLabel = SketchProofVertexLabel<Kind>;
    using Edge = std::tuple<VertexIndex, VertexIndex, SketchProofEdgeLabel>;

    auto graph_indices = urd::Store<urd::map<urd::index<tyr::planning::State<Kind>>, VertexIndex>, urd::dense> {};
    graph_indices.reserve(nodes.size());
    auto vertices = urd::Store<urd::sequence<VertexLabel>> {};
    result.deadend_states.clear();
    result.open_states.clear();
    const auto add_vertex = [&](const tyr::planning::PackedStateView<Kind>& state)
    {
        const auto [entry, inserted] = graph_indices.try_emplace(state.index(), static_cast<VertexIndex>(vertices.size()));
        const auto vertex = entry->second;
        if (!inserted)
            return vertex;
        const auto& node = nodes.at(state.index());
        if (node.is_deadend)
            result.deadend_states.push_back(vertex);
        if (node.is_open)
            result.open_states.push_back(vertex);
        vertices.emplace_back(state, initial == state, node.is_goal, !node.is_unsolvable, node.is_unsolvable);
        return vertex;
    };
    if (initial)
        add_vertex(*initial);

    auto edges = urd::Store<urd::sequence<Edge>> {};
    edges.reserve(predecessors.size());
    for (const auto& predecessor : predecessors)
    {
        const auto source = add_vertex(predecessor.source);
        const auto target = add_vertex(predecessor.target);
        edges.emplace_back(source, target, SketchProofEdgeLabel(predecessor.action, predecessor.rule));
    }
    result.graph = std::make_shared<SketchProofGraph<Kind>>(urd::graph_data<Edge, vertex_label_t<SketchProofGraph<Kind>>> { vertices.size(), edges, {}, vertices });
    result.cycle = vefr::find_vertex_cycle(*result.graph);
}

}  // namespace runir::kr::ps::base::detail

#endif
