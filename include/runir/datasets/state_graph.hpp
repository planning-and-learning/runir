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

#include "runir/datasets/config.hpp"
#include "runir/datasets/task_class.hpp"
#include "runir/graphs/bidirectional_static_graph.hpp"
#include "runir/graphs/dynamic_graph.hpp"
#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"

#include <chrono>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <tuple>
#include <tyr/planning/algorithms/utils.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::datasets
{

template<tyr::TaskKind Kind>
struct StateGraphVertexLabel : ygg::comparison::Mixin<StateGraphVertexLabel<Kind>>
{
    tyr::planning::StateView<Kind> state;

    explicit StateGraphVertexLabel(tyr::planning::StateView<Kind> state_) noexcept : state(std::move(state_)) {}

    auto cista_members() noexcept { return std::tie(state); }
    auto identifying_members() const noexcept { return std::tie(state); }
};

template<tyr::TaskKind Kind>
struct AnnotatedStateGraphVertexLabel : ygg::comparison::Mixin<AnnotatedStateGraphVertexLabel<Kind>>
{
    tyr::planning::StateView<Kind> state;
    ygg::float_t goal_distance = std::numeric_limits<ygg::float_t>::infinity();
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;

    AnnotatedStateGraphVertexLabel(tyr::planning::StateView<Kind> state_,
                                   ygg::float_t goal_distance_,
                                   bool is_initial_,
                                   bool is_goal_,
                                   bool is_alive_,
                                   bool is_unsolvable_) noexcept :
        state(std::move(state_)),
        goal_distance(goal_distance_),
        is_initial(is_initial_),
        is_goal(is_goal_),
        is_alive(is_alive_),
        is_unsolvable(is_unsolvable_)
    {
    }

    auto cista_members() noexcept { return std::tie(state, goal_distance, is_initial, is_goal, is_alive, is_unsolvable); }
    auto identifying_members() const noexcept { return std::tie(state, goal_distance, is_initial, is_goal, is_alive, is_unsolvable); }
};

struct StateGraphEdgeLabel : ygg::comparison::Mixin<StateGraphEdgeLabel>
{
    tyr::formalism::planning::ActionBindingView action;
    ygg::float_t cost = 0;

    StateGraphEdgeLabel() = delete;
    StateGraphEdgeLabel(tyr::formalism::planning::ActionBindingView action_, ygg::float_t cost_) noexcept : action(action_), cost(cost_) {}

    auto cista_members() noexcept { return std::tie(action, cost); }
    auto identifying_members() const noexcept { return std::make_tuple(action.get_index(), cost); }
};

template<tyr::TaskKind Kind>
using StateGraphBuilder = graphs::StaticGraphBuilder<StateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::TaskKind Kind>
using StaticStateGraph = graphs::StaticGraph<StateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::TaskKind Kind>
using StateGraph = graphs::BidirectionalStaticGraph<StateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::TaskKind Kind>
using DynamicStateGraph = graphs::DynamicGraph<StateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::TaskKind Kind>
using AnnotatedStateGraphBuilder = graphs::StaticGraphBuilder<AnnotatedStateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::TaskKind Kind>
using StaticAnnotatedStateGraph = graphs::StaticGraph<AnnotatedStateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::TaskKind Kind>
using AnnotatedStateGraph = graphs::BidirectionalStaticGraph<AnnotatedStateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

template<tyr::TaskKind Kind>
using DynamicAnnotatedStateGraph = graphs::DynamicGraph<AnnotatedStateGraphVertexLabel<Kind>, StateGraphEdgeLabel>;

struct StateGraphGenerationOptions
{
    ygg::uint_t max_num_states = std::numeric_limits<ygg::uint_t>::max();
    std::optional<std::chrono::steady_clock::duration> max_time = std::nullopt;
    std::size_t num_search_workers = 1;
};

template<tyr::TaskKind Kind>
struct StateGraphGenerationResult
{
    std::unique_ptr<StateGraph<Kind>> graph;
    tyr::planning::SearchStatus status = tyr::planning::SearchStatus::IN_PROGRESS;
};

template<tyr::TaskKind Kind>
auto generate_state_graph_result(TaskSearchContext<Kind>& context,
                                 const StateGraphGenerationOptions& options = StateGraphGenerationOptions()) -> StateGraphGenerationResult<Kind>;

template<tyr::TaskKind Kind>
auto generate_state_graph(TaskSearchContext<Kind>& context,
                          const StateGraphGenerationOptions& options = StateGraphGenerationOptions()) -> std::unique_ptr<StateGraph<Kind>>;

template<tyr::TaskKind Kind>
auto annotate_state_graph(TaskSearchContext<Kind>& context,
                          const StateGraph<Kind>& graph,
                          StateGraphCostMode cost_mode) -> std::unique_ptr<AnnotatedStateGraph<Kind>>;

}  // namespace runir::datasets

#endif
