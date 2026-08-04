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

#include "runir/datasets/state_graph.hpp"

#include "runir/graphs/bgl/algorithms.hpp"
#include "state_graph_fragment.hpp"

#include <cassert>
#include <fmt/format.h>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <tyr/planning/algorithms/astar_eager.hpp>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/heuristics/blind.hpp>
#include <utility>
#include <vector>

namespace runir::datasets
{

namespace
{

template<typename G>
auto compute_goal_distances(const G& graph, const std::vector<graphs::VertexIndex>& goal_vertices, StateGraphCostMode cost_mode)
{
    switch (cost_mode)
    {
        case StateGraphCostMode::UNIT_COST:
        {
            const auto [predecessors, distances] = graphs::bgl::breadth_first_search(graph, goal_vertices);
            static_cast<void>(predecessors);

            auto result = std::vector<ygg::float_t>(distances.size(), std::numeric_limits<ygg::float_t>::infinity());
            for (graphs::VertexIndex vertex = 0; vertex < distances.size(); ++vertex)
                if (distances[vertex] != std::numeric_limits<ygg::uint_t>::max())
                    result[vertex] = static_cast<ygg::float_t>(distances[vertex]);
            return result;
        }
        case StateGraphCostMode::ACTION_COST:
        {
            auto weights = std::vector<ygg::float_t>(graph.get_num_edges(), 0);
            for (auto edge : graph.get_edge_indices())
                weights[edge] = graph.get_edge(edge).get_property().cost;

            const auto [predecessors, distances] = graphs::bgl::dijkstra_shortest_paths(graph, weights, goal_vertices);
            static_cast<void>(predecessors);
            return distances;
        }
    }

    throw std::runtime_error(fmt::format("Unsupported state graph cost mode: {}.", static_cast<int>(cost_mode)));
}

}  // namespace

template<tyr::TaskKind Kind>
auto create_astar_options(const StateGraphGenerationOptions& generation_options)
{
    auto options = tyr::planning::astar_eager::Options<Kind> {};
    options.max_num_states = generation_options.max_num_states;
    options.max_time = generation_options.max_time;
    options.num_search_workers = generation_options.num_search_workers;
    return options;
}

template<tyr::TaskKind Kind>
auto generate_state_graph_result(TaskSearchContext<Kind>& context, const StateGraphGenerationOptions& generation_options) -> StateGraphGenerationResult<Kind>
{
    detail::validate_num_search_workers(generation_options.num_search_workers);

    auto heuristic = tyr::planning::BlindHeuristic<Kind> {};
    auto event_handler = std::make_shared<detail::StateGraphEventHandler<Kind>>(generation_options.num_search_workers);
    auto options = create_astar_options<Kind>(generation_options);
    options.event_handler = event_handler;
    options.goal_strategy = tyr::planning::ExhaustiveGoalStrategy<Kind>::create();

    const auto result = tyr::planning::astar_eager::find_solution(*context.task, *context.successor_generator, heuristic, options);

    return { detail::build_state_graph(*event_handler, *context.state_repository), result.status };
}

template<tyr::TaskKind Kind>
auto generate_state_graph(TaskSearchContext<Kind>& context, const StateGraphGenerationOptions& generation_options) -> std::unique_ptr<StateGraph<Kind>>
{
    return generate_state_graph_result(context, generation_options).graph;
}

template<tyr::TaskKind Kind>
auto annotate_state_graph(TaskSearchContext<Kind>& context,
                          const StateGraph<Kind>& graph,
                          StateGraphCostMode cost_mode) -> std::unique_ptr<AnnotatedStateGraph<Kind>>
{
    const auto& forward_graph = graph.get_forward_graph();
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*context.task);
    const auto static_goal_satisfied = goal_strategy.is_static_goal_satisfied(*context.task);
    const auto initial_state = context.successor_generator->get_initial_node().get_state();

    auto is_goal = std::vector<bool>(forward_graph.get_num_vertices(), false);
    auto goal_vertices = std::vector<graphs::VertexIndex> {};

    for (auto vertex_index : forward_graph.get_vertex_indices())
    {
        const auto& state = forward_graph.get_vertex(vertex_index).get_property().state;
        is_goal[vertex_index] = static_goal_satisfied && goal_strategy.is_dynamic_goal_satisfied(initial_state, state);
        if (is_goal[vertex_index])
            goal_vertices.push_back(vertex_index);
    }

    const auto goal_distance = compute_goal_distances(graph.get_backward_graph(), goal_vertices, cost_mode);

    auto builder = AnnotatedStateGraphBuilder<Kind> {};
    for (auto vertex_index : forward_graph.get_vertex_indices())
    {
        const auto& state = forward_graph.get_vertex(vertex_index).get_property().state;
        const auto is_alive = goal_distance[vertex_index] != std::numeric_limits<ygg::float_t>::infinity();
        [[maybe_unused]] const auto added = builder.add_vertex(
            AnnotatedStateGraphVertexLabel<Kind> { state, goal_distance[vertex_index], state == initial_state, is_goal[vertex_index], is_alive, !is_alive });
        assert(added == vertex_index);
    }

    for (auto edge_index : forward_graph.get_edge_indices())
    {
        const auto& edge = forward_graph.get_edge(edge_index);
        builder.add_directed_edge(edge.get_source(), edge.get_target(), edge.get_property());
    }

    return std::make_unique<AnnotatedStateGraph<Kind>>(std::move(builder));
}

template auto generate_state_graph_result<tyr::GroundTag>(TaskSearchContext<tyr::GroundTag>&,
                                                          const StateGraphGenerationOptions&) -> StateGraphGenerationResult<tyr::GroundTag>;

template auto generate_state_graph_result<tyr::LiftedTag>(TaskSearchContext<tyr::LiftedTag>&,
                                                          const StateGraphGenerationOptions&) -> StateGraphGenerationResult<tyr::LiftedTag>;

template auto generate_state_graph<tyr::GroundTag>(TaskSearchContext<tyr::GroundTag>&,
                                                   const StateGraphGenerationOptions&) -> std::unique_ptr<StateGraph<tyr::GroundTag>>;

template auto generate_state_graph<tyr::LiftedTag>(TaskSearchContext<tyr::LiftedTag>&,
                                                   const StateGraphGenerationOptions&) -> std::unique_ptr<StateGraph<tyr::LiftedTag>>;

template auto annotate_state_graph<tyr::GroundTag>(TaskSearchContext<tyr::GroundTag>&,
                                                   const StateGraph<tyr::GroundTag>&,
                                                   StateGraphCostMode) -> std::unique_ptr<AnnotatedStateGraph<tyr::GroundTag>>;

template auto annotate_state_graph<tyr::LiftedTag>(TaskSearchContext<tyr::LiftedTag>&,
                                                   const StateGraph<tyr::LiftedTag>&,
                                                   StateGraphCostMode) -> std::unique_ptr<AnnotatedStateGraph<tyr::LiftedTag>>;

}  // namespace runir::datasets
