#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_GRAPH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_GRAPH_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/ps/ext/detail/search_node.hpp"
#include "runir/kr/ps/ext/detail/search_path.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <span>
#include <tuple>
#include <urd/repr/maps/index_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <urd/config.hpp>

namespace runir::kr::ps::ext::detail
{

/// Materialize every recorded transition in discovery order, preserving parallel edges and rejected branches.
/// Open/deadend/cycle diagnostics describe the explored graph; they do not override the search's AND/OR result.
template<tyr::TaskKind Kind>
void build_proof_graph(ProgramProofResults<Kind>& result,
                       const SearchNodes<Kind>& nodes,
                       const urd::Store<urd::sequence<Predecessor<Kind>>>& predecessors,
                       std::optional<ProgramStateView<Kind>> initial)
{
    using VertexLabel = ProgramProofVertexLabel<ExtFamilyTag, Kind>;
    using Edge = std::tuple<graphs::VertexIndex, graphs::VertexIndex, ProgramProofEdgeLabel<ExtFamilyTag>>;

    auto graph_indices = urd::Store<urd::map<urd::index<ProgramState<Kind>>, graphs::VertexIndex>, urd::dense> {};
    graph_indices.reserve(nodes.size());
    auto vertices = urd::Store<urd::sequence<VertexLabel>> {};
    result.deadend_states.clear();
    result.open_states.clear();
    const auto add_vertex = [&](ProgramStateView<Kind> state)
    {
        const auto [entry, inserted] = graph_indices.try_emplace(state.index(), static_cast<graphs::VertexIndex>(vertices.size()));
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
        auto label = ProgramProofEdgeLabel<ExtFamilyTag> {};
        if (predecessor.action)
            label.action = *predecessor.action;
        if (predecessor.rule)
            label.rule = *predecessor.rule;
        edges.emplace_back(source, target, std::move(label));
    }
    result.graph = std::make_shared<ProgramProofGraph<ExtFamilyTag, Kind>>(std::span<const VertexLabel>(vertices), std::span<const Edge>(edges));
    result.cycle = graphs::find_cycle(*result.graph);
}

/// Materialize only a selected reached path; it is diagnostic, not a complete AND/OR proof.
template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
void build_witness_graph(ProgramProofResults<Kind>& result,
                         SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>& expander,
                         const std::shared_ptr<SearchPath<Kind, ExecutionStorage>>& witness)
{
    using PathPtr = std::shared_ptr<SearchPath<Kind, ExecutionStorage>>;
    auto path = urd::Store<urd::sequence<PathPtr>> {};
    for (auto current = witness; current; current = current->parent)
        path.push_back(current);
    std::ranges::reverse(path);
    using Vertex = ProgramProofVertexLabel<ExtFamilyTag, Kind>;
    using Edge = std::tuple<graphs::VertexIndex, graphs::VertexIndex, ProgramProofEdgeLabel<ExtFamilyTag>>;
    auto vertices = urd::Store<urd::sequence<Vertex>> {};
    auto edges = urd::Store<urd::sequence<Edge>> {};
    vertices.reserve(path.size());
    for (std::size_t i = 0; i < path.size(); ++i)
    {
        const auto& entry = *path[i];
        const auto index = static_cast<graphs::VertexIndex>(i);
        // Retained paths are initialized; only released pool entries lack a state.
        URD_ASSUME(entry.state.has_value());
        vertices.emplace_back(expander.materialize(expander.view(*entry.state)), i == 0, entry.is_goal, !entry.is_unsolvable, entry.is_unsolvable);
        if (entry.is_deadend)
            result.deadend_states.push_back(index);
        if (entry.is_open)
            result.open_states.push_back(index);
        if (i)
        {
            auto label = ProgramProofEdgeLabel<ExtFamilyTag> {};
            if (entry.transition)
                label.action = entry.transition->action;
            if (entry.rule)
                label.rule = *entry.rule;
            edges.emplace_back(index - 1, index, std::move(label));
        }
    }
    result.graph = std::make_shared<ProgramProofGraph<ExtFamilyTag, Kind>>(std::span<const Vertex>(vertices), std::span<const Edge>(edges));
}

}  // namespace runir::kr::ps::ext::detail

#endif
