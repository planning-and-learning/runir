/*
 * Copyright (C) 2025-2026 Dominik Drexler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef RUNIR_DATASETS_EQUIVALENCE_GRAPH_HPP_
#define RUNIR_DATASETS_EQUIVALENCE_GRAPH_HPP_

#include "runir/datasets/config.hpp"
#include "runir/datasets/equivalence_policy.hpp"
#include "runir/datasets/graph_types.hpp"
#include "runir/datasets/object_graph.hpp"
#include "runir/datasets/state_graph.hpp"
#include "runir/datasets/task_class.hpp"

#include <limits>
#include <memory>
#include <tuple>
#include <urd/repr/graphs/csr_graph.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace runir::datasets
{

struct EquivalenceVertexLabel : urd::compared_by_identity<EquivalenceVertexLabel>
{
    ygg::uint_t state_graph_index = 0;
    VertexIndex state_vertex_index = 0;

    EquivalenceVertexLabel() = default;
    constexpr EquivalenceVertexLabel(ygg::uint_t state_graph_index_, VertexIndex state_vertex_index_) noexcept :
        state_graph_index(state_graph_index_),
        state_vertex_index(state_vertex_index_)
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&EquivalenceVertexLabel::state_graph_index, &EquivalenceVertexLabel::state_vertex_index> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&EquivalenceVertexLabel::state_graph_index, &EquivalenceVertexLabel::state_vertex_index> {}; }
};

struct AnnotatedEquivalenceVertexLabel : urd::compared_by_identity<AnnotatedEquivalenceVertexLabel>
{
    ygg::uint_t state_graph_index = 0;
    VertexIndex state_vertex_index = 0;
    ygg::float_t goal_distance = std::numeric_limits<ygg::float_t>::infinity();
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    AnnotatedEquivalenceVertexLabel() = default;
    AnnotatedEquivalenceVertexLabel(ygg::uint_t state_graph_index_,
                                    VertexIndex state_vertex_index_,
                                    ygg::float_t goal_distance_,
                                    bool is_initial_,
                                    bool is_goal_,
                                    bool is_alive_,
                                    bool is_unsolvable_) noexcept :
        state_graph_index(state_graph_index_),
        state_vertex_index(state_vertex_index_),
        goal_distance(goal_distance_),
        is_initial(is_initial_),
        is_goal(is_goal_),
        is_alive(is_alive_),
        is_unsolvable(is_unsolvable_)
    {
    }

    static constexpr auto cista_members() noexcept
    {
        return urd::Members<&AnnotatedEquivalenceVertexLabel::state_graph_index,
                            &AnnotatedEquivalenceVertexLabel::state_vertex_index,
                            &AnnotatedEquivalenceVertexLabel::goal_distance,
                            &AnnotatedEquivalenceVertexLabel::is_initial,
                            &AnnotatedEquivalenceVertexLabel::is_goal,
                            &AnnotatedEquivalenceVertexLabel::is_alive,
                            &AnnotatedEquivalenceVertexLabel::is_unsolvable> {};
    }

    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&AnnotatedEquivalenceVertexLabel::state_graph_index,
                            &AnnotatedEquivalenceVertexLabel::state_vertex_index,
                            &AnnotatedEquivalenceVertexLabel::goal_distance,
                            &AnnotatedEquivalenceVertexLabel::is_initial,
                            &AnnotatedEquivalenceVertexLabel::is_goal,
                            &AnnotatedEquivalenceVertexLabel::is_alive,
                            &AnnotatedEquivalenceVertexLabel::is_unsolvable> {};
    }
};

struct EquivalenceEdgeLabel : urd::compared_by_identity<EquivalenceEdgeLabel>
{
    ygg::uint_t state_graph_index = 0;
    EdgeIndex state_edge_index = 0;

    EquivalenceEdgeLabel() = default;
    constexpr EquivalenceEdgeLabel(ygg::uint_t state_graph_index_, EdgeIndex state_edge_index_) noexcept :
        state_graph_index(state_graph_index_),
        state_edge_index(state_edge_index_)
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&EquivalenceEdgeLabel::state_graph_index, &EquivalenceEdgeLabel::state_edge_index> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&EquivalenceEdgeLabel::state_graph_index, &EquivalenceEdgeLabel::state_edge_index> {}; }
};

/// The classes of states the equivalence policy finds across tasks, each represented by a state of a state graph, and an
/// edge between two classes where a transition leads from a state of one to a state of the other (labelled by the first
/// such transition, by its number in its state graph): a frozen graph that keeps its in-edges.
using EquivalenceGraph = urd::Store<urd::graph<VertexIndex, EquivalenceEdgeLabel, EquivalenceVertexLabel>, urd::frozen, urd::bidirectional>;

/// An equivalence graph whose classes know their goal distance and whether they are initial, goal, alive or unsolvable.
using AnnotatedEquivalenceGraph =
    urd::Store<urd::graph<VertexIndex, EquivalenceEdgeLabel, AnnotatedEquivalenceVertexLabel>, urd::frozen, urd::bidirectional>;
// The concepts it is bound and searched by (urd/graphs.hpp): a renamed member is a compile error, not a lost feature.
static_assert(urd::bidirectional_graph<EquivalenceGraph> && urd::edge_list_graph<EquivalenceGraph>);
static_assert(urd::bidirectional_graph<AnnotatedEquivalenceGraph> && urd::edge_list_graph<AnnotatedEquivalenceGraph>);

template<tyr::TaskKind Kind>
struct EquivalenceGraphConstructionResult
{
    urd::Store<urd::sequence<StateGraphGenerationResult<Kind>>> state_graph_results;
    std::unique_ptr<EquivalenceGraph> graph;
};

struct EquivalenceGraphGenerationOptions
{
    StateGraphGenerationOptions state_graph_options;
    EquivalencePolicyMode policy_mode = EquivalencePolicyMode::IDENTITY;
};

template<tyr::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
auto generate_equivalence_graph(TaskSearchContextList<Kind>& contexts,
                                Policy& policy,
                                const StateGraphGenerationOptions& state_graph_options = StateGraphGenerationOptions())
    -> EquivalenceGraphConstructionResult<Kind>;

template<tyr::TaskKind Kind>
auto generate_equivalence_graph(TaskSearchContextList<Kind>& contexts, const EquivalenceGraphGenerationOptions& options, ColorRepository& color_repository)
    -> EquivalenceGraphConstructionResult<Kind>;

}  // namespace runir::datasets

#endif
