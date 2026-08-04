/*
 * Copyright (C) 2026 Dominik Drexler
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

#ifndef RUNIR_SRC_DATASETS_STATE_GRAPH_FRAGMENT_HPP_
#define RUNIR_SRC_DATASETS_STATE_GRAPH_FRAGMENT_HPP_

#include "runir/datasets/state_graph.hpp"

#include <cassert>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <tyr/planning/algorithms/astar_eager/event_handler.hpp>
#include <tyr/planning/state_repository.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::datasets::detail
{

using StateGraphEdgeKey = std::tuple<graphs::VertexIndex, graphs::VertexIndex, tyr::formalism::planning::ActionBindingView, ygg::float_t>;

inline void validate_num_search_workers(size_t num_workers)
{
    if (num_workers == 0)
        throw std::invalid_argument("State graph generation requires at least one search worker.");
    if (num_workers > std::numeric_limits<ygg::uint_t>::max())
        throw std::invalid_argument("State graph generation worker count exceeds the worker index range.");
}

template<tyr::TaskKind Kind>
struct StateLocator : ygg::comparison::Mixin<StateLocator<Kind>>
{
    ygg::uint_t repository;
    ygg::Index<tyr::planning::State<Kind>> state;

    StateLocator(ygg::uint_t repository_, ygg::Index<tyr::planning::State<Kind>> state_) : repository(repository_), state(state_) {}

    auto identifying_members() const noexcept { return std::tie(repository, state); }
};

template<tyr::TaskKind Kind>
struct StateGraphTransition
{
    StateLocator<Kind> source;
    StateLocator<Kind> target;
    tyr::formalism::planning::ActionBindingView action;
    ygg::float_t cost;
};

template<tyr::TaskKind Kind>
class StateGraphFragment
{
public:
    using RepositoryMap = ygg::UnorderedMap<ygg::uint_t, tyr::planning::StateRepositoryPtr<Kind>>;

    StateLocator<Kind> locate(const tyr::planning::StateView<Kind>& state)
    {
        const auto& repository = state.get_state_repository();
        const auto index = repository->get_index();
        [[maybe_unused]] const auto [it, inserted] = m_repositories.emplace(index, repository);
        assert(inserted || it->second.get() == repository.get());
        return { index, state.get_index() };
    }

    void record(const tyr::planning::Node<Kind>& source, const tyr::planning::LabeledNode<Kind>& target)
    {
        m_transitions.push_back(StateGraphTransition<Kind> { locate(source.get_state()),
                                                             locate(target.node.get_state()),
                                                             target.label,
                                                             target.node.get_metric() - source.get_metric() });
    }

    void append_repositories(RepositoryMap& repositories) const
    {
        for (const auto& [index, repository] : m_repositories)
        {
            [[maybe_unused]] const auto [it, inserted] = repositories.emplace(index, repository);
            assert(inserted || it->second.get() == repository.get());
        }
    }

    const auto& get_transitions() const noexcept { return m_transitions; }

private:
    RepositoryMap m_repositories;
    std::vector<StateGraphTransition<Kind>> m_transitions;
};

template<tyr::TaskKind Kind>
class StateGraphWorkerEventHandler final : public tyr::planning::astar_eager::WorkerEventHandler<Kind>
{
public:
    explicit StateGraphWorkerEventHandler(StateGraphFragment<Kind>& fragment) : m_fragment(&fragment) {}

    void
    on_generate_transition(const tyr::planning::Node<Kind>& source, const tyr::planning::LabeledNode<Kind>& target, tyr::planning::TransitionOutcome) override
    {
        m_fragment->record(source, target);
    }

private:
    StateGraphFragment<Kind>* m_fragment;
};

template<tyr::TaskKind Kind>
class StateGraphEventHandler final : public tyr::planning::astar_eager::EventHandler<Kind>
{
public:
    explicit StateGraphEventHandler(size_t num_workers) : m_workers(num_workers) {}

    void on_start_search(const tyr::planning::Node<Kind>& node, ygg::float_t) override
    {
        assert(!m_start);
        m_start = m_root.locate(node.get_state());
    }

    void on_end_search(tyr::planning::SearchStatus, const tyr::planning::Statistics&) override {}
    void on_solved(const tyr::planning::Plan<Kind>&) override {}

    auto make_worker(ygg::Index<tyr::planning::Worker> index) -> tyr::planning::astar_eager::WorkerEventHandlerPtr<Kind> override
    {
        const auto value = static_cast<size_t>(ygg::uint_t(index));
        if (value >= m_workers.size())
            throw std::out_of_range("State graph worker index is out of range.");
        return std::make_unique<StateGraphWorkerEventHandler<Kind>>(m_workers[value]);
    }

    StateGraphFragment<Kind>& get_worker(ygg::Index<tyr::planning::Worker> index)
    {
        const auto value = static_cast<size_t>(ygg::uint_t(index));
        assert(value < m_workers.size());
        return m_workers[value];
    }

    const auto& get_workers() const noexcept { return m_workers; }
    const auto& get_start() const noexcept { return m_start; }

    auto collect_repositories() const -> typename StateGraphFragment<Kind>::RepositoryMap
    {
        auto repositories = typename StateGraphFragment<Kind>::RepositoryMap {};
        m_root.append_repositories(repositories);
        for (const auto& worker : m_workers)
            worker.append_repositories(repositories);
        return repositories;
    }

private:
    StateGraphFragment<Kind> m_root;
    std::vector<StateGraphFragment<Kind>> m_workers;
    std::optional<StateLocator<Kind>> m_start;
};

template<tyr::TaskKind Kind>
tyr::planning::StateView<Kind> materialize_state(const StateLocator<Kind>& locator,
                                                 const typename StateGraphFragment<Kind>::RepositoryMap& repositories,
                                                 tyr::planning::StateRepository<Kind>& target)
{
    const auto it = repositories.find(locator.repository);
    if (it == repositories.end())
        throw std::logic_error("State graph fragment references an unknown state repository.");
    return tyr::planning::materialize_state(it->second->get_registered_state(locator.state), target);
}

template<tyr::TaskKind Kind>
std::unique_ptr<StateGraph<Kind>> build_state_graph(const StateGraphEventHandler<Kind>& events, tyr::planning::StateRepository<Kind>& target_repository)
{
    if (!events.get_start())
        throw std::logic_error("State graph search did not report its initial state.");

    const auto repositories = events.collect_repositories();
    auto builder = StateGraphBuilder<Kind> {};
    auto state_to_vertex = ygg::UnorderedMap<tyr::planning::StateView<Kind>, graphs::VertexIndex> {};
    auto edges = ygg::UnorderedSet<StateGraphEdgeKey> {};
    auto get_or_create_vertex = [&](tyr::planning::StateView<Kind> state)
    {
        if (const auto it = state_to_vertex.find(state); it != state_to_vertex.end())
            return it->second;

        const auto vertex = static_cast<graphs::VertexIndex>(state_to_vertex.size());
        state_to_vertex.emplace(state, vertex);
        [[maybe_unused]] const auto added = builder.add_vertex(StateGraphVertexLabel<Kind> { std::move(state) });
        assert(added == vertex);
        return vertex;
    };

    static_cast<void>(get_or_create_vertex(materialize_state(*events.get_start(), repositories, target_repository)));
    for (const auto& worker : events.get_workers())
    {
        for (const auto& transition : worker.get_transitions())
        {
            const auto source = get_or_create_vertex(materialize_state(transition.source, repositories, target_repository));
            const auto target = get_or_create_vertex(materialize_state(transition.target, repositories, target_repository));
            if (edges.emplace(source, target, transition.action, transition.cost).second)
                builder.add_directed_edge(source, target, StateGraphEdgeLabel { transition.action, transition.cost });
        }
    }

    return std::make_unique<StateGraph<Kind>>(std::move(builder));
}

}  // namespace runir::datasets::detail

#endif
