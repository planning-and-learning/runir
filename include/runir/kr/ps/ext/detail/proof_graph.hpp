#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_GRAPH_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/ps/ext/detail/search_node.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/containers/segmented_vector.hpp>
#include <yggdrasil/containers/unordered_set.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
void build_proof_graph(ProgramProofResults<Kind>& result,
                       const ygg::SegmentedVector<SearchNode<Kind>>& nodes,
                       const ygg::UnorderedSet<Predecessor<Kind>>& predecessors,
                       std::optional<ygg::Index<ProgramState<Kind>>> initial)
{
    using VertexLabel = ProgramProofVertexLabel<Kind>;
    using Edge = std::tuple<graphs::VertexIndex, graphs::VertexIndex, ProgramProofEdgeLabel>;
    constexpr auto no_vertex = std::numeric_limits<graphs::VertexIndex>::max();
    constexpr auto unreached = SearchNode<Kind>::unreached;

    auto graph_indices = std::vector<graphs::VertexIndex>(nodes.size(), no_vertex);
    auto states = std::vector<ygg::Index<ProgramState<Kind>>> {};
    auto records = std::vector<const Predecessor<Kind>*> {};
    for (std::size_t i = 0; i < nodes.size(); ++i)
        if (nodes[i].step != unreached)
            states.emplace_back(static_cast<ygg::uint_t>(i));
    for (const auto& predecessor : predecessors)
        records.push_back(&predecessor);

    std::ranges::sort(states, {}, [&](auto state) { return nodes[ygg::uint_t(state)].step; });
    std::ranges::sort(records, {}, [](const auto* predecessor) { return predecessor->order; });
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
    edges.reserve(records.size());
    for (const auto* predecessor : records)
    {
        auto label = ProgramProofEdgeLabel {};
        if (predecessor->action)
            label.state_transition = ProgramProofStateTransition(*predecessor->action, predecessor->cost);
        if (predecessor->rule)
            label.rule = *predecessor->rule;
        edges.emplace_back(graph_indices[ygg::uint_t(predecessor->source)], graph_indices[ygg::uint_t(predecessor->target)], std::move(label));
    }
    result.graph = std::make_shared<ProgramProofGraph<Kind>>(std::span<const VertexLabel>(vertices), std::span<const Edge>(edges));
    result.cycle = graphs::find_cycle(*result.graph);
}

}  // namespace runir::kr::ps::ext::detail

#endif
