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

#ifndef RUNIR_DATASETS_STATE_GRAPH_HPP_
#define RUNIR_DATASETS_STATE_GRAPH_HPP_

#include "runir/common/config.hpp"
#include "runir/datasets/config.hpp"
#include "runir/datasets/equivalence_policy.hpp"
#include "runir/datasets/task_class.hpp"
#include "runir/graphs/bidirectional_static_graph.hpp"
#include "runir/graphs/dynamic_graph.hpp"
#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"

#include <limits>
#include <memory>
#include <tuple>
#include <tyr/planning/planning.hpp>

namespace runir::datasets
{

template<tyr::planning::TaskKind Kind>
struct StateGraphVertexLabel
{
    tyr::planning::StateView<Kind> state;

    auto identifying_members() const noexcept { return std::tie(state); }
};

template<tyr::planning::TaskKind Kind>
struct AnnotatedStateGraphVertexLabel
{
    tyr::planning::StateView<Kind> state;
    tyr::float_t goal_distance = std::numeric_limits<tyr::float_t>::infinity();
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    auto identifying_members() const noexcept { return std::tie(state, goal_distance, is_initial, is_goal, is_alive, is_unsolvable); }
};

struct StateGraphEdgeLabel
{
    tyr::formalism::planning::GroundActionView action;
    tyr::float_t cost = 0;

    auto identifying_members() const noexcept { return std::make_tuple(action.get_index(), cost); }
};

template<tyr::planning::TaskKind Kind>
using StateGraphBuilder = graphs::StaticGraphBuilder<StateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using StaticStateGraph = graphs::StaticGraph<StateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using StateGraph = graphs::BidirectionalStaticGraph<StateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using DynamicStateGraph = graphs::DynamicGraph<StateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using AnnotatedStateGraphBuilder = graphs::StaticGraphBuilder<AnnotatedStateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using StaticAnnotatedStateGraph = graphs::StaticGraph<AnnotatedStateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using AnnotatedStateGraph = graphs::BidirectionalStaticGraph<AnnotatedStateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::planning::TaskKind Kind>
using DynamicAnnotatedStateGraph = graphs::DynamicGraph<AnnotatedStateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::planning::TaskKind Kind>
auto generate_state_graph(TaskSearchContext<Kind>& context) -> std::unique_ptr<StateGraph<Kind>>;

template<tyr::planning::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
auto generate_state_graph(TaskSearchContext<Kind>& context, uint_t state_graph_index, Policy& policy) -> std::unique_ptr<StateGraph<Kind>>;

template<tyr::planning::TaskKind Kind>
auto annotate_state_graph(TaskSearchContext<Kind>& context, const StateGraph<Kind>& graph, StateGraphCostMode cost_mode)
    -> std::unique_ptr<AnnotatedStateGraph<Kind>>;

}  // namespace runir::datasets

#endif
