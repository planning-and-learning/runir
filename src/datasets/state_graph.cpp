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

#include "runir/datasets/equivalence_policies/identity.hpp"
#include "runir/graphs/bgl/algorithms.hpp"

#include <cassert>
#include <limits>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

namespace runir::datasets
{

namespace
{

template<tyr::planning::TaskKind Kind>
class StateGraphEventHandler : public tyr::planning::astar_eager::EventHandler<Kind>
{
private:
    StateGraphBuilder<Kind> m_builder;
    tyr::UnorderedMap<tyr::planning::StateView<Kind>, graphs::VertexIndex> m_state_to_vertex;
    tyr::planning::Statistics m_statistics;

    auto get_or_create_vertex(tyr::planning::Node<Kind> node) -> graphs::VertexIndex
    {
        const auto state = node.get_state();
        if (const auto it = m_state_to_vertex.find(state); it != m_state_to_vertex.end())
            return it->second;

        const auto vertex = static_cast<graphs::VertexIndex>(m_state_to_vertex.size());
        m_state_to_vertex.emplace(state, vertex);
        [[maybe_unused]] const auto added = m_builder.add_vertex(StateGraphVertexLabel<Kind> { state });
        assert(added == vertex);
        return vertex;
    }

    void record_edge(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node)
    {
        const auto source = get_or_create_vertex(source_node);
        const auto target = get_or_create_vertex(labeled_succ_node.node);
        const auto cost = labeled_succ_node.node.get_metric() - source_node.get_metric();
        m_builder.add_directed_edge(source, target, StateGraphEdgeLabel { labeled_succ_node.label, cost });
    }

public:
    void on_expand_node(const tyr::planning::Node<Kind>& node) override { get_or_create_vertex(node); }

    void on_expand_goal_node(const tyr::planning::Node<Kind>& node) override { static_cast<void>(node); }

    void on_generate_node(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node) override
    {
        record_edge(source_node, labeled_succ_node);
    }

    void on_generate_node_relaxed(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node) override
    {
        static_cast<void>(source_node);
        static_cast<void>(labeled_succ_node);
    }

    void on_generate_node_not_relaxed(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node) override
    {
        record_edge(source_node, labeled_succ_node);
    }

    void on_close_node(const tyr::planning::Node<Kind>& node) override { static_cast<void>(node); }

    void on_prune_node(const tyr::planning::Node<Kind>& node) override { static_cast<void>(node); }

    void on_prune_node(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node) override
    {
        record_edge(source_node, labeled_succ_node);
    }

    void on_start_search(const tyr::planning::Node<Kind>& node, tyr::float_t f_value) override
    {
        static_cast<void>(f_value);
        get_or_create_vertex(node);
    }

    void on_finish_f_layer(tyr::float_t f_value) override { static_cast<void>(f_value); }

    void on_end_search() override {}

    void on_end_search(tyr::planning::SearchStatus status) override { static_cast<void>(status); }

    void on_solved(const tyr::planning::Plan<Kind>& plan) override { static_cast<void>(plan); }

    void on_unsolvable() override {}

    void on_exhausted() override {}

    const tyr::planning::Statistics& get_search_statistics() const override { return m_statistics; }

    const tyr::planning::Statistics& get_statistics() const override { return m_statistics; }

    auto release() && { return std::make_unique<StateGraph<Kind>>(std::move(m_builder)); }
};

template<tyr::planning::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
class PolicyStateGraphPruningStrategy : public tyr::planning::PruningStrategy<Kind>
{
private:
    uint_t m_state_graph_index;
    Policy* m_policy;

public:
    PolicyStateGraphPruningStrategy(uint_t state_graph_index, Policy& policy) : m_state_graph_index(state_graph_index), m_policy(&policy) {}

    bool should_prune_state(const tyr::planning::StateView<Kind>& state) override
    {
        return !m_policy->try_insert(StateGraphVertexCandidate<Kind> { m_state_graph_index, state });
    }

    bool should_prune_successor_state(const tyr::planning::StateView<Kind>& state, const tyr::planning::StateView<Kind>& succ_state, bool is_new_succ) override
    {
        if (!is_new_succ)
            return false;

        return !m_policy->try_insert(StateGraphTransitionCandidate<Kind> { m_state_graph_index, state, succ_state });
    }
};

template<typename G>
auto compute_goal_distances(const G& graph, const std::vector<graphs::VertexIndex>& goal_vertices, StateGraphCostMode cost_mode)
{
    switch (cost_mode)
    {
        case StateGraphCostMode::UNIT_COST:
        {
            const auto [predecessors, distances] = graphs::bgl::breadth_first_search(graph, goal_vertices);
            static_cast<void>(predecessors);

            auto result = std::vector<tyr::float_t>(distances.size(), std::numeric_limits<tyr::float_t>::infinity());
            for (graphs::VertexIndex vertex = 0; vertex < distances.size(); ++vertex)
                if (distances[vertex] != std::numeric_limits<uint_t>::max())
                    result[vertex] = static_cast<tyr::float_t>(distances[vertex]);
            return result;
        }
        case StateGraphCostMode::ACTION_COST:
        {
            auto weights = std::vector<tyr::float_t>(graph.get_num_edges(), 0);
            for (auto edge : graph.get_edge_indices())
                weights[edge] = graph.get_edge(edge).get_property().cost;

            const auto [predecessors, distances] = graphs::bgl::dijkstra_shortest_paths(graph, weights, goal_vertices);
            static_cast<void>(predecessors);
            return distances;
        }
    }

    assert(false);
    return std::vector<tyr::float_t>(graph.get_num_vertices(), std::numeric_limits<tyr::float_t>::infinity());
}

}  // namespace

template<tyr::planning::TaskKind Kind>
auto generate_state_graph(TaskSearchContext<Kind>& context) -> std::unique_ptr<StateGraph<Kind>>
{
    auto heuristic = tyr::planning::BlindHeuristic<Kind> {};
    auto event_handler = std::make_shared<StateGraphEventHandler<Kind>>();
    auto options = tyr::planning::astar_eager::Options<Kind> {};
    options.event_handler = event_handler;
    options.goal_strategy = tyr::planning::ExhaustiveGoalStrategy<Kind>::create();
    options.pruning_strategy = tyr::planning::PruningStrategy<Kind>::create();

    [[maybe_unused]] const auto result = tyr::planning::astar_eager::find_solution(*context.task, *context.successor_generator, heuristic, options);

    return std::move(*event_handler).release();
}

template<tyr::planning::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
auto generate_state_graph(TaskSearchContext<Kind>& context, uint_t state_graph_index, Policy& policy) -> std::unique_ptr<StateGraph<Kind>>
{
    auto heuristic = tyr::planning::BlindHeuristic<Kind> {};
    auto event_handler = std::make_shared<StateGraphEventHandler<Kind>>();
    auto options = tyr::planning::astar_eager::Options<Kind> {};
    options.event_handler = event_handler;
    options.goal_strategy = tyr::planning::ExhaustiveGoalStrategy<Kind>::create();
    options.pruning_strategy = std::make_shared<PolicyStateGraphPruningStrategy<Kind, Policy>>(state_graph_index, policy);

    [[maybe_unused]] const auto result = tyr::planning::astar_eager::find_solution(*context.task, *context.successor_generator, heuristic, options);

    return std::move(*event_handler).release();
}

template<tyr::planning::TaskKind Kind>
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
        const auto is_alive = goal_distance[vertex_index] != std::numeric_limits<tyr::float_t>::infinity();
        [[maybe_unused]] const auto added =
            builder.add_vertex(AnnotatedStateGraphVertexLabel<Kind> { state,
                                                                      goal_distance[vertex_index],
                                                                      tyr::EqualTo<tyr::planning::StateView<Kind>> {}(state, initial_state),
                                                                      is_goal[vertex_index],
                                                                      is_alive,
                                                                      !is_alive });
        assert(added == vertex_index);
    }

    for (auto edge_index : forward_graph.get_edge_indices())
    {
        const auto& edge = forward_graph.get_edge(edge_index);
        builder.add_directed_edge(edge.get_source(), edge.get_target(), edge.get_property());
    }

    return std::make_unique<AnnotatedStateGraph<Kind>>(std::move(builder));
}

template auto
generate_state_graph<tyr::planning::GroundTag>(TaskSearchContext<tyr::planning::GroundTag>&) -> std::unique_ptr<StateGraph<tyr::planning::GroundTag>>;

template auto
generate_state_graph<tyr::planning::LiftedTag>(TaskSearchContext<tyr::planning::LiftedTag>&) -> std::unique_ptr<StateGraph<tyr::planning::LiftedTag>>;

template auto generate_state_graph<tyr::planning::GroundTag, EquivalencePolicy<IdentityEquivalenceTag>>(TaskSearchContext<tyr::planning::GroundTag>&,
                                                                                                        uint_t,
                                                                                                        EquivalencePolicy<IdentityEquivalenceTag>&)
    -> std::unique_ptr<StateGraph<tyr::planning::GroundTag>>;

template auto generate_state_graph<tyr::planning::LiftedTag, EquivalencePolicy<IdentityEquivalenceTag>>(TaskSearchContext<tyr::planning::LiftedTag>&,
                                                                                                        uint_t,
                                                                                                        EquivalencePolicy<IdentityEquivalenceTag>&)
    -> std::unique_ptr<StateGraph<tyr::planning::LiftedTag>>;

template auto annotate_state_graph<tyr::planning::GroundTag>(TaskSearchContext<tyr::planning::GroundTag>&,
                                                             const StateGraph<tyr::planning::GroundTag>&,
                                                             StateGraphCostMode) -> std::unique_ptr<AnnotatedStateGraph<tyr::planning::GroundTag>>;

template auto annotate_state_graph<tyr::planning::LiftedTag>(TaskSearchContext<tyr::planning::LiftedTag>&,
                                                             const StateGraph<tyr::planning::LiftedTag>&,
                                                             StateGraphCostMode) -> std::unique_ptr<AnnotatedStateGraph<tyr::planning::LiftedTag>>;

}  // namespace runir::datasets
