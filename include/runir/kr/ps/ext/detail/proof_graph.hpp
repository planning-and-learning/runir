#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_GRAPH_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/ps/ext/detail/search_node.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"
#include "runir/kr/task_context.hpp"

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

template<tyr::TaskKind Kind>
void build_proof_graph(ProgramProofResults<Kind>& result,
                       const ygg::SegmentedVector<SearchNode<Kind>>& nodes,
                       const std::vector<Predecessor<Kind>>& predecessors,
                       std::optional<ygg::Index<ProgramState<Kind>>> initial)
{
    using VertexLabel = ProgramProofVertexLabel<Kind>;
    using Edge = std::tuple<graphs::VertexIndex, graphs::VertexIndex, ProgramProofEdgeLabel>;
    constexpr auto no_vertex = std::numeric_limits<graphs::VertexIndex>::max();

    auto graph_indices = std::vector<graphs::VertexIndex>(nodes.size(), no_vertex);
    auto states = std::vector<ygg::Index<ProgramState<Kind>>> {};
    if (initial)
        states.push_back(*initial);
    for (std::size_t i = 0; i < nodes.size(); ++i)
        if (nodes[i].parent_state != ygg::Index<ProgramState<Kind>>::max())
            states.emplace_back(static_cast<ygg::uint_t>(i));
    auto vertices = std::vector<VertexLabel> {};
    vertices.reserve(states.size());
    result.deadend_states.clear();
    result.open_states.clear();
    for (const auto state : states)
    {
        const auto vertex = static_cast<graphs::VertexIndex>(vertices.size());
        graph_indices[ygg::uint_t(state)] = vertex;
        const auto& node = nodes[ygg::uint_t(state)];
        if (node.is_deadend)
            result.deadend_states.push_back(vertex);
        if (node.is_open)
            result.open_states.push_back(vertex);
        vertices.emplace_back(ProgramStateView<Kind>(state, *result.task_context_owner->execution_repository),
                              initial && *initial == state,
                              node.is_goal,
                              !node.is_unsolvable,
                              node.is_unsolvable);
    }
    auto edges = std::vector<Edge> {};
    edges.reserve(predecessors.size());
    for (const auto& predecessor : predecessors)
    {
        auto label = ProgramProofEdgeLabel {};
        if (predecessor.action)
            label.state_transition = ProgramProofStateTransition(*predecessor.action, ygg::float_t(1));
        if (predecessor.rule)
            label.rule = *predecessor.rule;
        edges.emplace_back(graph_indices[ygg::uint_t(predecessor.source)], graph_indices[ygg::uint_t(predecessor.target)], std::move(label));
    }
    result.graph = std::make_shared<ProgramProofGraph<Kind>>(std::span<const VertexLabel>(vertices), std::span<const Edge>(edges));
    result.cycle = graphs::find_cycle(*result.graph);
}

}  // namespace runir::kr::ps::ext::detail

#endif
