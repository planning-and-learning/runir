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

#include "runir/datasets/equivalence_graph.hpp"

#include "runir/datasets/equivalence_policies/gi.hpp"
#include "runir/datasets/equivalence_policies/identity.hpp"
#include "state_graph_fragment.hpp"

#include <atomic>
#include <cassert>
#include <fmt/format.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <tyr/planning/algorithms/astar_eager.hpp>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/algorithms/strategies/pruning.hpp>
#include <tyr/planning/heuristics/blind.hpp>
#include <utility>
#include <vector>

namespace runir::datasets
{

namespace
{

using RepresentativeToVertexMap = ygg::UnorderedMap<EquivalenceVertexLabel, graphs::VertexIndex>;
using EquivalenceEdgeSet = ygg::UnorderedSet<std::pair<graphs::VertexIndex, graphs::VertexIndex>>;

struct StateAssignment
{
    std::optional<StateGraphVertexRef> concrete;
    StateGraphVertexRef representative;
};

template<tyr::TaskKind Kind>
struct EquivalenceWorkerState
{
    ygg::UnorderedMap<detail::StateLocator<Kind>, StateAssignment> assignments;
};

template<tyr::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
class EquivalenceCoordinator
{
public:
    EquivalenceCoordinator(ygg::uint_t state_graph_index, Policy& policy, size_t num_workers) :
        m_state_graph_index(state_graph_index),
        m_policy(&policy),
        m_workers(num_workers)
    {
    }

    void initialize_start(const tyr::planning::StateView<Kind>& state, detail::StateLocator<Kind> locator)
    {
        assert(!m_start);
        const auto concrete = allocate_vertex();
        const auto result =
            m_policy->get_or_create_representative(StateGraphVertexCandidate<Kind> { m_state_graph_index, state }, [concrete] { return concrete; });
        assert(!result.inserted || result.representative == concrete);
        m_start = std::pair { locator, StateAssignment { concrete, result.representative } };
    }

    bool register_start(ygg::Index<tyr::planning::Worker> worker, detail::StateLocator<Kind> locator)
    {
        if (!m_start)
            throw std::logic_error("Equivalence search did not report its initial state before pruning.");
        auto& assignments = get_worker(worker).assignments;
        [[maybe_unused]] const auto [it, inserted] = assignments.emplace(locator, m_start->second);
        assert(inserted || (it->second.concrete == m_start->second.concrete && it->second.representative == m_start->second.representative));
        return false;
    }

    bool should_prune_successor(ygg::Index<tyr::planning::Worker> worker,
                                const tyr::planning::StateView<Kind>& state,
                                detail::StateLocator<Kind> locator,
                                bool is_new)
    {
        auto& assignments = get_worker(worker).assignments;
        if (const auto it = assignments.find(locator); it != assignments.end())
            return !it->second.concrete;

        if (!is_new)
            throw std::logic_error("Equivalence pruning is missing an assignment for a previously encountered state.");

        const auto result =
            m_policy->get_or_create_representative(StateGraphVertexCandidate<Kind> { m_state_graph_index, state }, [this] { return allocate_vertex(); });
        const auto assignment = StateAssignment { result.inserted ? std::optional(result.representative) : std::nullopt, result.representative };
        assignments.emplace(locator, assignment);
        return !assignment.concrete;
    }

    size_t get_num_state_vertices() const noexcept { return static_cast<size_t>(m_next_state_vertex.load(std::memory_order_relaxed)); }
    const auto& get_start() const noexcept { return m_start; }
    const auto& get_workers() const noexcept { return m_workers; }

private:
    StateGraphVertexRef allocate_vertex()
    {
        return { m_state_graph_index, static_cast<graphs::VertexIndex>(m_next_state_vertex.fetch_add(1, std::memory_order_relaxed)) };
    }

    EquivalenceWorkerState<Kind>& get_worker(ygg::Index<tyr::planning::Worker> worker)
    {
        const auto index = static_cast<size_t>(ygg::uint_t(worker));
        assert(index < m_workers.size());
        return m_workers[index];
    }

    ygg::uint_t m_state_graph_index;
    Policy* m_policy;
    std::atomic<ygg::uint_t> m_next_state_vertex { 0 };
    std::vector<EquivalenceWorkerState<Kind>> m_workers;
    std::optional<std::pair<detail::StateLocator<Kind>, StateAssignment>> m_start;
};

template<tyr::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
class EquivalenceGraphEventHandler final : public tyr::planning::astar_eager::EventHandler<Kind>
{
public:
    EquivalenceGraphEventHandler(size_t num_workers, std::shared_ptr<EquivalenceCoordinator<Kind, Policy>> coordinator) :
        m_events(num_workers),
        m_coordinator(std::move(coordinator))
    {
    }

    void on_start_search(const tyr::planning::Node<Kind>& node, ygg::float_t f_value) override
    {
        m_events.on_start_search(node, f_value);
        m_coordinator->initialize_start(node.get_state(), *m_events.get_start());
    }

    void on_end_search(tyr::planning::SearchStatus, const tyr::planning::Statistics&) override {}
    void on_solved(const tyr::planning::Plan<Kind>&) override {}

    auto make_worker(ygg::Index<tyr::planning::Worker> index) -> tyr::planning::astar_eager::WorkerEventHandlerPtr<Kind> override
    {
        return m_events.make_worker(index);
    }

    detail::StateGraphEventHandler<Kind>& get_events() noexcept { return m_events; }
    const detail::StateGraphEventHandler<Kind>& get_events() const noexcept { return m_events; }

private:
    detail::StateGraphEventHandler<Kind> m_events;
    std::shared_ptr<EquivalenceCoordinator<Kind, Policy>> m_coordinator;
};

template<tyr::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
class EquivalenceGraphPruningStrategy final : public tyr::planning::PruningStrategy<Kind>
{
public:
    EquivalenceGraphPruningStrategy(std::shared_ptr<EquivalenceCoordinator<Kind, Policy>> coordinator,
                                    detail::StateGraphEventHandler<Kind>& events,
                                    ygg::Index<tyr::planning::Worker> worker = ygg::Index<tyr::planning::Worker>(0)) :
        m_coordinator(std::move(coordinator)),
        m_events(&events),
        m_worker(worker)
    {
    }

    [[nodiscard]] tyr::planning::PruningStrategyPtr<Kind> make_worker(ygg::Index<tyr::planning::Worker> worker) const override
    {
        return std::make_shared<EquivalenceGraphPruningStrategy>(m_coordinator, *m_events, worker);
    }

    bool should_prune_state(const tyr::planning::StateView<Kind>& state) override
    {
        return m_coordinator->register_start(m_worker, m_events->get_worker(m_worker).locate(state));
    }

    bool should_prune_successor_state(const tyr::planning::StateView<Kind>&, const tyr::planning::StateView<Kind>& successor, bool is_new) override
    {
        return m_coordinator->should_prune_successor(m_worker, successor, m_events->get_worker(m_worker).locate(successor), is_new);
    }

private:
    std::shared_ptr<EquivalenceCoordinator<Kind, Policy>> m_coordinator;
    detail::StateGraphEventHandler<Kind>* m_events;
    ygg::Index<tyr::planning::Worker> m_worker;
};

template<tyr::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
std::unique_ptr<StateGraph<Kind>> build_state_graph(TaskSearchContext<Kind>& context,
                                                    ygg::uint_t state_graph_index,
                                                    const EquivalenceCoordinator<Kind, Policy>& coordinator,
                                                    const detail::StateGraphEventHandler<Kind>& events,
                                                    EquivalenceGraphBuilder& equivalence_builder,
                                                    RepresentativeToVertexMap& representative_to_vertex,
                                                    EquivalenceEdgeSet& equivalence_edges)
{
    if (!coordinator.get_start())
        throw std::logic_error("Equivalence search did not report its initial state.");

    const auto repositories = events.collect_repositories();
    auto assignments = ygg::UnorderedMap<detail::StateLocator<Kind>, StateAssignment> {};
    auto concrete_states = std::vector<std::optional<detail::StateLocator<Kind>>>(coordinator.get_num_state_vertices());

    const auto& [start_locator, start_assignment] = *coordinator.get_start();
    assignments.emplace(start_locator, start_assignment);
    assert(start_assignment.concrete);
    concrete_states[start_assignment.concrete->state_vertex_index] = start_locator;

    for (const auto& worker : coordinator.get_workers())
    {
        for (const auto& [locator, assignment] : worker.assignments)
        {
            assignments.emplace(locator, assignment);
            if (assignment.concrete && !concrete_states[assignment.concrete->state_vertex_index])
                concrete_states[assignment.concrete->state_vertex_index] = locator;
        }
    }

    const auto get_assignment = [&](detail::StateLocator<Kind> locator) -> const StateAssignment&
    {
        const auto it = assignments.find(locator);
        if (it == assignments.end())
            throw std::logic_error("Equivalence graph transition references a state without an assignment.");
        return it->second;
    };

    const auto get_or_create_equivalence_vertex = [&](StateGraphVertexRef representative)
    {
        const auto label = EquivalenceVertexLabel { representative.state_graph_index, representative.state_vertex_index };
        if (const auto it = representative_to_vertex.find(label); it != representative_to_vertex.end())
            return it->second;

        const auto vertex = static_cast<graphs::VertexIndex>(representative_to_vertex.size());
        representative_to_vertex.emplace(label, vertex);
        [[maybe_unused]] const auto added = equivalence_builder.add_vertex(label);
        assert(added == vertex);
        return vertex;
    };

    auto state_builder = StateGraphBuilder<Kind> {};
    auto state_edges = ygg::UnorderedSet<detail::StateGraphEdgeKey> {};
    auto state_to_equivalence = std::vector<graphs::VertexIndex> {};
    state_to_equivalence.reserve(concrete_states.size());
    for (graphs::VertexIndex vertex = 0; vertex < concrete_states.size(); ++vertex)
    {
        if (!concrete_states[vertex])
            throw std::logic_error("Equivalence graph construction left a hole in the state vertex indices.");
        auto state = detail::materialize_state(*concrete_states[vertex], repositories, *context.state_repository);
        [[maybe_unused]] const auto added = state_builder.add_vertex(StateGraphVertexLabel<Kind> { std::move(state) });
        assert(added == vertex);
        state_to_equivalence.push_back(get_or_create_equivalence_vertex(get_assignment(*concrete_states[vertex]).representative));
    }

    for (const auto& worker : events.get_workers())
    {
        for (const auto& transition : worker.get_transitions())
        {
            const auto& source_assignment = get_assignment(transition.source);
            const auto& target_assignment = get_assignment(transition.target);
            if (!source_assignment.concrete)
                throw std::logic_error("Equivalence graph transition source was pruned.");

            const auto target = target_assignment.concrete.value_or(target_assignment.representative);
            if (target.state_graph_index != state_graph_index)
                throw std::runtime_error("Cannot create a concrete state edge to a representative in a different state graph.");

            if (!state_edges.emplace(source_assignment.concrete->state_vertex_index, target.state_vertex_index, transition.action, transition.cost).second)
                continue;

            const auto state_edge = state_builder.add_directed_edge(source_assignment.concrete->state_vertex_index,
                                                                    target.state_vertex_index,
                                                                    StateGraphEdgeLabel { transition.action, transition.cost });
            const auto equivalence_source = state_to_equivalence[source_assignment.concrete->state_vertex_index];
            const auto equivalence_target = get_or_create_equivalence_vertex(target_assignment.representative);
            if (equivalence_edges.emplace(equivalence_source, equivalence_target).second)
                equivalence_builder.add_directed_edge(equivalence_source, equivalence_target, EquivalenceEdgeLabel { state_graph_index, state_edge });
        }
    }

    return std::make_unique<StateGraph<Kind>>(std::move(state_builder));
}

template<tyr::TaskKind Kind>
auto create_astar_options(const StateGraphGenerationOptions& options)
{
    auto result = tyr::planning::astar_eager::Options<Kind> {};
    result.max_num_states = options.max_num_states;
    result.max_time = options.max_time;
    result.num_search_workers = options.num_search_workers;
    return result;
}

}  // namespace

template<tyr::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
auto generate_equivalence_graph(TaskSearchContextList<Kind>& contexts,
                                Policy& policy,
                                const StateGraphGenerationOptions& state_graph_options) -> EquivalenceGraphConstructionResult<Kind>
{
    detail::validate_num_search_workers(state_graph_options.num_search_workers);

    auto equivalence_builder = EquivalenceGraphBuilder {};
    auto representative_to_vertex = RepresentativeToVertexMap {};
    auto equivalence_edges = EquivalenceEdgeSet {};
    auto state_graph_results = std::vector<StateGraphGenerationResult<Kind>> {};
    state_graph_results.reserve(contexts.size());

    for (ygg::uint_t state_graph_index = 0; state_graph_index < contexts.size(); ++state_graph_index)
    {
        auto& context = *contexts[state_graph_index];
        auto heuristic = tyr::planning::BlindHeuristic<Kind> {};
        auto coordinator = std::make_shared<EquivalenceCoordinator<Kind, Policy>>(state_graph_index, policy, state_graph_options.num_search_workers);
        auto event_handler = std::make_shared<EquivalenceGraphEventHandler<Kind, Policy>>(state_graph_options.num_search_workers, coordinator);
        auto pruning_strategy = std::make_shared<EquivalenceGraphPruningStrategy<Kind, Policy>>(coordinator, event_handler->get_events());
        auto options = create_astar_options<Kind>(state_graph_options);
        options.event_handler = event_handler;
        options.goal_strategy = tyr::planning::ExhaustiveGoalStrategy<Kind>::create();
        options.pruning_strategy = std::move(pruning_strategy);

        const auto result = tyr::planning::astar_eager::find_solution(*context.task, *context.successor_generator, heuristic, options);
        auto state_graph = build_state_graph(context,
                                             state_graph_index,
                                             *coordinator,
                                             event_handler->get_events(),
                                             equivalence_builder,
                                             representative_to_vertex,
                                             equivalence_edges);
        state_graph_results.push_back(StateGraphGenerationResult<Kind> { std::move(state_graph), result.status });
    }

    return { std::move(state_graph_results), std::make_unique<EquivalenceGraph>(std::move(equivalence_builder)) };
}

template<tyr::TaskKind Kind>
auto generate_equivalence_graph(TaskSearchContextList<Kind>& contexts, EquivalencePolicyMode policy_mode) -> EquivalenceGraphConstructionResult<Kind>
{
    return generate_equivalence_graph(contexts, EquivalenceGraphGenerationOptions { StateGraphGenerationOptions(), policy_mode });
}

template<tyr::TaskKind Kind>
auto generate_equivalence_graph(TaskSearchContextList<Kind>& contexts,
                                const EquivalenceGraphGenerationOptions& options) -> EquivalenceGraphConstructionResult<Kind>
{
    switch (options.policy_mode)
    {
        case EquivalencePolicyMode::IDENTITY:
        {
            auto policy = EquivalencePolicy<IdentityEquivalenceTag> {};
            return generate_equivalence_graph(contexts, policy, options.state_graph_options);
        }
        case EquivalencePolicyMode::GI:
        {
            auto policy = EquivalencePolicy<GIEquivalenceTag> {};
            return generate_equivalence_graph(contexts, policy, options.state_graph_options);
        }
    }

    throw std::runtime_error(fmt::format("Unsupported equivalence policy mode: {}.", static_cast<int>(options.policy_mode)));
}

template auto generate_equivalence_graph<tyr::GroundTag, EquivalencePolicy<IdentityEquivalenceTag>>(TaskSearchContextList<tyr::GroundTag>&,
                                                                                                    EquivalencePolicy<IdentityEquivalenceTag>&,
                                                                                                    const StateGraphGenerationOptions&)
    -> EquivalenceGraphConstructionResult<tyr::GroundTag>;

template auto generate_equivalence_graph<tyr::LiftedTag, EquivalencePolicy<IdentityEquivalenceTag>>(TaskSearchContextList<tyr::LiftedTag>&,
                                                                                                    EquivalencePolicy<IdentityEquivalenceTag>&,
                                                                                                    const StateGraphGenerationOptions&)
    -> EquivalenceGraphConstructionResult<tyr::LiftedTag>;

template auto generate_equivalence_graph<tyr::GroundTag, EquivalencePolicy<GIEquivalenceTag>>(TaskSearchContextList<tyr::GroundTag>&,
                                                                                              EquivalencePolicy<GIEquivalenceTag>&,
                                                                                              const StateGraphGenerationOptions&)
    -> EquivalenceGraphConstructionResult<tyr::GroundTag>;

template auto generate_equivalence_graph<tyr::LiftedTag, EquivalencePolicy<GIEquivalenceTag>>(TaskSearchContextList<tyr::LiftedTag>&,
                                                                                              EquivalencePolicy<GIEquivalenceTag>&,
                                                                                              const StateGraphGenerationOptions&)
    -> EquivalenceGraphConstructionResult<tyr::LiftedTag>;

template auto generate_equivalence_graph<tyr::GroundTag>(TaskSearchContextList<tyr::GroundTag>&,
                                                         EquivalencePolicyMode) -> EquivalenceGraphConstructionResult<tyr::GroundTag>;

template auto generate_equivalence_graph<tyr::LiftedTag>(TaskSearchContextList<tyr::LiftedTag>&,
                                                         EquivalencePolicyMode) -> EquivalenceGraphConstructionResult<tyr::LiftedTag>;

template auto generate_equivalence_graph<tyr::GroundTag>(TaskSearchContextList<tyr::GroundTag>&,
                                                         const EquivalenceGraphGenerationOptions&) -> EquivalenceGraphConstructionResult<tyr::GroundTag>;

template auto generate_equivalence_graph<tyr::LiftedTag>(TaskSearchContextList<tyr::LiftedTag>&,
                                                         const EquivalenceGraphGenerationOptions&) -> EquivalenceGraphConstructionResult<tyr::LiftedTag>;

}  // namespace runir::datasets
