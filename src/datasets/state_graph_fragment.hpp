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
#include <memory>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <tyr/planning/algorithms/astar_eager/event_handler.hpp>
#include <tyr/planning/state_repository.hpp>
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/indexed.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>

namespace runir::datasets::detail
{

using StateGraphEdgeKey = std::tuple<VertexIndex, VertexIndex, tyr::formalism::planning::ActionRowView, ygg::float_t>;

template<tyr::TaskKind Kind>
struct StateGraphTransition
{
    tyr::planning::PackedStateView<Kind> source;
    tyr::planning::PackedStateView<Kind> target;
    tyr::formalism::planning::ActionRowView action;
    ygg::float_t cost = 0;
};

template<tyr::TaskKind Kind>
class StateGraphFragment
{
public:
    void record(const tyr::planning::Node<Kind>& source, const tyr::planning::LabeledNode<Kind>& target)
    {
        m_transitions.push_back(StateGraphTransition<Kind> { source.get_state().pack(),
                                                             target.node.get_state().pack(),
                                                             target.label,
                                                             target.node.get_metric() - source.get_metric() });
    }

    const auto& get_transitions() const noexcept { return m_transitions; }

private:
    urd::Store<urd::sequence<StateGraphTransition<Kind>>> m_transitions;
};

template<tyr::TaskKind Kind>
class StateGraphWorkerEventHandler final : public tyr::planning::astar_eager::WorkerEventHandler<Kind>
{
public:
    explicit StateGraphWorkerEventHandler(StateGraphFragment<Kind>& fragment) : m_fragment(&fragment) {}

    void
    on_generate_transition(const tyr::planning::Node<Kind>& source, const tyr::planning::LabeledNode<Kind>& target, tyr::planning::TransitionOutcome) override
    { m_fragment->record(source, target); }

private:
    StateGraphFragment<Kind>* m_fragment;
};

template<tyr::TaskKind Kind>
class StateGraphEventHandler final : public tyr::planning::astar_eager::EventHandler<Kind>
{
public:
    explicit StateGraphEventHandler(const tyr::planning::StateView<Kind>& start) : m_workers(1), m_start(start.pack()) {}

    void on_start_search([[maybe_unused]] const tyr::planning::Node<Kind>& node, ygg::float_t) override { assert(m_start == node.get_state().pack()); }

    void on_end_search(tyr::planning::SearchStatus, const tyr::planning::Statistics&) override {}
    void on_solved(const tyr::planning::Plan<Kind>&) override {}

    auto make_worker(urd::index<tyr::planning::Worker> index) -> tyr::planning::astar_eager::WorkerEventHandlerPtr<Kind> override
    {
        const auto value = static_cast<size_t>(static_cast<ygg::uint_t>(index.value));
        if (value >= m_workers.size())
            throw std::out_of_range("State graph worker index is out of range.");
        return std::make_unique<StateGraphWorkerEventHandler<Kind>>(m_workers[value]);
    }

    const auto& get_workers() const noexcept { return m_workers; }
    const auto& get_start() const noexcept { return m_start; }

private:
    urd::Store<urd::sequence<StateGraphFragment<Kind>>> m_workers;
    std::optional<tyr::planning::PackedStateView<Kind>> m_start;
};

template<tyr::TaskKind Kind>
std::unique_ptr<StateGraph<Kind>> build_state_graph(const StateGraphEventHandler<Kind>& events,
                                                    tyr::planning::StateRepository<Kind>& target_repository,
                                                    tyr::planning::AxiomEvaluator<Kind>& axiom_evaluator)
{
    if (!events.get_start())
        throw std::logic_error("State graph search did not report its initial state.");

    using Graph = StateGraph<Kind>;
    using Edge = std::tuple<VertexIndex, VertexIndex, StateGraphEdgeLabel>;
    auto vertices = urd::Store<urd::sequence<StateGraphVertexLabel<Kind>, VertexIndex>> {};
    auto graph_edges = urd::Store<urd::sequence<Edge>> {};
    auto packed_to_vertex = urd::Store<urd::map<tyr::planning::PackedStateView<Kind>, VertexIndex>> {};
    /// The materialized states by vertex: a state's vertex is its position.
    auto states = urd::Store<urd::set<tyr::planning::PackedStateView<Kind>>, urd::positional>(urd::layer { 0 });
    auto edges = urd::Store<urd::set<StateGraphEdgeKey>> {};
    auto get_or_create_vertex = [&](const tyr::planning::PackedStateView<Kind>& packed)
    {
        if (const auto it = packed_to_vertex.find(packed); it != packed_to_vertex.end())
            return it->second;

        auto state = tyr::planning::materialize_state(packed.unpack(), target_repository, axiom_evaluator).pack();
        const auto [it, inserted] = states.insert(state);
        const auto vertex = static_cast<VertexIndex>(states.position(it));
        if (inserted)
        {
            assert(vertices.size() == vertex);
            vertices.push_back(StateGraphVertexLabel<Kind> { std::move(state) });
        }
        packed_to_vertex.emplace(packed, vertex);
        return vertex;
    };

    static_cast<void>(get_or_create_vertex(*events.get_start()));
    for (const auto& worker : events.get_workers())
    {
        for (const auto& transition : worker.get_transitions())
        {
            const auto source = get_or_create_vertex(transition.source);
            const auto target = get_or_create_vertex(transition.target);
            if (edges.emplace(source, target, transition.action, transition.cost).second)
                graph_edges.emplace_back(source, target, StateGraphEdgeLabel { transition.action, transition.cost });
        }
    }

    return std::make_unique<Graph>(urd::graph_data<Edge, StateGraphVertexLabel<Kind>> { vertices.size(), graph_edges, {}, vertices });
}

}  // namespace runir::datasets::detail

#endif
