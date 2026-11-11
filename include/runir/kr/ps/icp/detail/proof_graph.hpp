#ifndef RUNIR_KR_PS_ICP_DETAIL_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_PROOF_GRAPH_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/ps/icp/detail/search_node.hpp"
#include "runir/kr/ps/icp/program_executor_data.hpp"

#include <cstddef>
#include <memory>
#include <span>
#include <tuple>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>

namespace runir::kr::ps::icp::detail
{

/// Materialize reached states and recorded transitions in discovery order, preserving parallel edges.
template<tyr::TaskKind Kind>
void build_proof_graph(ProgramProofResults<Kind>& result, const urd::Store<urd::sequence<SearchNode<Kind>>>& nodes, std::span<const Predecessor> predecessors)
{
    using VertexLabel = ProgramProofVertexLabel<IcpFamilyTag, Kind>;
    using Edge = std::tuple<graphs::VertexIndex, graphs::VertexIndex, ProgramProofEdgeLabel<IcpFamilyTag>>;

    auto vertices = urd::Store<urd::sequence<VertexLabel>> {};
    vertices.reserve(nodes.size());
    result.deadend_states.clear();
    result.open_states.clear();
    for (std::size_t i = 0; i < nodes.size(); ++i)
    {
        const auto& node = nodes[i];
        vertices.emplace_back(node.state, !node.parent, node.is_goal, !node.is_unsolvable, node.is_unsolvable);
        if (node.is_deadend)
            result.deadend_states.push_back(i);
        if (node.is_open)
            result.open_states.push_back(i);
    }
    auto edges = urd::Store<urd::sequence<Edge>> {};
    edges.reserve(predecessors.size());
    for (const auto& predecessor : predecessors)
    {
        auto label = ProgramProofEdgeLabel<IcpFamilyTag> {};
        if (predecessor.action)
            label.action = *predecessor.action;
        if (predecessor.rule)
            label.rule = *predecessor.rule;
        edges.emplace_back(predecessor.source, predecessor.target, std::move(label));
    }
    result.graph = std::make_shared<ProgramProofGraph<IcpFamilyTag, Kind>>(std::span<const VertexLabel>(vertices), std::span<const Edge>(edges));
    result.cycle = graphs::find_cycle(*result.graph);
}

}  // namespace runir::kr::ps::icp::detail

#endif
