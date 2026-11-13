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

#include "state_graph_fragment.hpp"

#include <cassert>
#include <fmt/format.h>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <tyr/planning/algorithms/astar_eager.hpp>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/heuristics/blind.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/dense_set.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <vefr/algorithms.hpp>
#include <vefr/reversed.hpp>

namespace runir::datasets
{

namespace
{

/// Each vertex's distance to a goal: searched backwards from the goal vertices, an edge costing one or its action's cost.
template<typename G>
auto compute_goal_distances(const G& graph, const VertexIndexList& goal_vertices, StateGraphCostMode cost_mode)
{
    const auto backward = vefr::reversed(graph);
    auto result = urd::Store<urd::sequence<ygg::float_t>>(graph.num_vertices(), std::numeric_limits<ygg::float_t>::infinity());
    switch (cost_mode)
    {
        case StateGraphCostMode::UNIT_COST:
        {
            const auto paths = vefr::breadth_first_search(backward, goal_vertices);
            for (const auto vertex : graph.vertices())
                if (const auto distance = paths.distances[vertex])
                    result[vertex] = static_cast<ygg::float_t>(*distance);
            return result;
        }
        case StateGraphCostMode::ACTION_COST:
        {
            const auto cost = [&graph](std::size_t edge) { return graph[edge].cost; };
            const auto paths = vefr::dijkstra_shortest_paths(backward, goal_vertices, cost);
            for (const auto vertex : graph.vertices())
                result[vertex] = paths.distances[vertex];
            return result;
        }
    }
    throw std::runtime_error(fmt::format("Unsupported state graph cost mode: {}.", static_cast<int>(cost_mode)));
}

template<tyr::TaskKind Kind>
auto create_astar_options(const StateGraphGenerationOptions& generation_options)
{
    auto options = tyr::planning::astar_eager::Options<Kind> {};
    options.search_budget = { generation_options.max_num_states, generation_options.max_time };
    options.num_search_workers = 1;
    return options;
}

}  // namespace

template<tyr::TaskKind Kind>
auto generate_state_graph_result(TaskSearchContext<Kind>& context, const StateGraphGenerationOptions& generation_options) -> StateGraphGenerationResult<Kind>
{
    auto heuristic = tyr::planning::BlindHeuristic<Kind> {};
    const auto initial_node = context.successor_generator->get_initial_node(*context.state_repository, *context.axiom_evaluator);
    auto event_handler = std::make_shared<detail::StateGraphEventHandler<Kind>>(initial_node.get_state());
    auto options = create_astar_options<Kind>(generation_options);
    options.event_handler = event_handler;
    options.goal_strategy = tyr::planning::ExhaustiveGoalStrategy<Kind>::create();

    const auto result = tyr::planning::astar_eager::find_solution(*context.task,
                                                                  *context.state_repository,
                                                                  *context.axiom_evaluator,
                                                                  *context.successor_generator,
                                                                  heuristic,
                                                                  options);

    return { detail::build_state_graph(*event_handler, *context.state_repository, *context.axiom_evaluator), result.status };
}

template<tyr::TaskKind Kind>
auto generate_state_graph(TaskSearchContext<Kind>& context, const StateGraphGenerationOptions& generation_options) -> std::unique_ptr<StateGraph<Kind>>
{ return generate_state_graph_result(context, generation_options).graph; }

template<tyr::TaskKind Kind>
auto annotate_state_graph(TaskSearchContext<Kind>& context, const StateGraph<Kind>& graph, StateGraphCostMode cost_mode)
    -> std::unique_ptr<AnnotatedStateGraph<Kind>>
{
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*context.task);
    const auto static_goal_satisfied = goal_strategy.is_static_goal_satisfied(*context.task);
    const auto initial_state = context.successor_generator->get_initial_node(*context.state_repository, *context.axiom_evaluator).get_state();
    const auto packed_initial_state = initial_state.pack();

    auto is_goal = urd::Store<urd::set<VertexIndex>, urd::dense> {};
    auto goal_vertices = VertexIndexList {};

    for (const auto vertex_index : graph.vertices())
    {
        const auto state = graph.vertex_value(vertex_index).state.unpack();
        if (static_goal_satisfied && goal_strategy.is_dynamic_goal_satisfied(initial_state, state))
        {
            is_goal.insert(vertex_index);
            goal_vertices.push_back(vertex_index);
        }
    }

    const auto goal_distance = compute_goal_distances(graph, goal_vertices, cost_mode);

    using Result = AnnotatedStateGraph<Kind>;
    auto vertices = urd::Store<urd::sequence<vertex_label_t<Result>, VertexIndex>> {};
    vertices.reserve(graph.num_vertices());
    for (const auto vertex_index : graph.vertices())
    {
        const auto& state = graph.vertex_value(vertex_index).state;
        const auto is_alive = goal_distance[vertex_index] != std::numeric_limits<ygg::float_t>::infinity();
        vertices.push_back(AnnotatedStateGraphVertexLabel<Kind> { state,
                                                                  goal_distance[vertex_index],
                                                                  state == packed_initial_state,
                                                                  is_goal.contains(vertex_index),
                                                                  is_alive,
                                                                  !is_alive });
    }

    // The same edges in the same order: the annotated graph numbers them as the state graph does.
    using Edge = std::tuple<VertexIndex, VertexIndex, StateGraphEdgeLabel>;
    auto edges = urd::Store<urd::sequence<Edge>> {};
    edges.reserve(graph.num_edges());
    for (const auto edge : graph.edges())
        edges.emplace_back(graph.source(edge), graph.target(edge), graph[edge]);
    return std::make_unique<Result>(urd::graph_data<Edge, vertex_label_t<Result>> { vertices.size(), edges, {}, vertices });
}

template auto generate_state_graph_result<tyr::GroundTag>(TaskSearchContext<tyr::GroundTag>&, const StateGraphGenerationOptions&)
    -> StateGraphGenerationResult<tyr::GroundTag>;

template auto generate_state_graph_result<tyr::LiftedTag>(TaskSearchContext<tyr::LiftedTag>&, const StateGraphGenerationOptions&)
    -> StateGraphGenerationResult<tyr::LiftedTag>;

template auto generate_state_graph<tyr::GroundTag>(TaskSearchContext<tyr::GroundTag>&, const StateGraphGenerationOptions&)
    -> std::unique_ptr<StateGraph<tyr::GroundTag>>;

template auto generate_state_graph<tyr::LiftedTag>(TaskSearchContext<tyr::LiftedTag>&, const StateGraphGenerationOptions&)
    -> std::unique_ptr<StateGraph<tyr::LiftedTag>>;

template auto annotate_state_graph<tyr::GroundTag>(TaskSearchContext<tyr::GroundTag>&, const StateGraph<tyr::GroundTag>&, StateGraphCostMode)
    -> std::unique_ptr<AnnotatedStateGraph<tyr::GroundTag>>;

template auto annotate_state_graph<tyr::LiftedTag>(TaskSearchContext<tyr::LiftedTag>&, const StateGraph<tyr::LiftedTag>&, StateGraphCostMode)
    -> std::unique_ptr<AnnotatedStateGraph<tyr::LiftedTag>>;

}  // namespace runir::datasets
