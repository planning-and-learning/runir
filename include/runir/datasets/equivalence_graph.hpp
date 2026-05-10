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

#include "runir/common/config.hpp"
#include "runir/datasets/config.hpp"
#include "runir/graphs/bidirectional_static_graph.hpp"
#include "runir/graphs/dynamic_graph.hpp"
#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"

#include <limits>
#include <tuple>

namespace runir::datasets
{

struct EquivalenceVertexLabel
{
    uint_t state_graph_index = 0;
    graphs::VertexIndex state_vertex_index;

    auto identifying_members() const noexcept { return std::tie(state_graph_index, state_vertex_index); }
};

struct AnnotatedEquivalenceVertexLabel
{
    uint_t state_graph_index = 0;
    graphs::VertexIndex state_vertex_index;
    tyr::float_t goal_distance = std::numeric_limits<tyr::float_t>::infinity();
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    auto identifying_members() const noexcept
    {
        return std::tie(state_graph_index, state_vertex_index, goal_distance, is_initial, is_goal, is_alive, is_unsolvable);
    }
};

struct EquivalenceEdgeLabel
{
    uint_t state_graph_index = 0;
    graphs::EdgeIndex state_edge_index;

    auto identifying_members() const noexcept { return std::tie(state_graph_index, state_edge_index); }
};

using EquivalenceGraphBuilder = graphs::StaticGraphBuilder<EquivalenceVertexLabel, EquivalenceEdgeLabel>;

using StaticEquivalenceGraph = graphs::StaticGraph<EquivalenceVertexLabel, EquivalenceEdgeLabel>;

using EquivalenceGraph = graphs::BidirectionalStaticGraph<EquivalenceVertexLabel, EquivalenceEdgeLabel>;

using DynamicEquivalenceGraph = graphs::DynamicGraph<EquivalenceVertexLabel, EquivalenceEdgeLabel>;

using AnnotatedEquivalenceGraphBuilder = graphs::StaticGraphBuilder<AnnotatedEquivalenceVertexLabel, EquivalenceEdgeLabel>;

using StaticAnnotatedEquivalenceGraph = graphs::StaticGraph<AnnotatedEquivalenceVertexLabel, EquivalenceEdgeLabel>;

using AnnotatedEquivalenceGraph = graphs::BidirectionalStaticGraph<AnnotatedEquivalenceVertexLabel, EquivalenceEdgeLabel>;

using DynamicAnnotatedEquivalenceGraph = graphs::DynamicGraph<AnnotatedEquivalenceVertexLabel, EquivalenceEdgeLabel>;

}  // namespace runir::datasets

#endif
