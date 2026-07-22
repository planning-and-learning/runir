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

#ifndef RUNIR_DATASETS_EQUIVALENCE_POLICY_HPP_
#define RUNIR_DATASETS_EQUIVALENCE_POLICY_HPP_

#include "runir/config.hpp"
#include "runir/graphs/declarations.hpp"

#include <concepts>
#include <tuple>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <utility>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::datasets
{

template<typename Tag>
struct EquivalencePolicy;

struct StateGraphVertexRef : ygg::comparison::Mixin<StateGraphVertexRef>
{
    uint_t state_graph_index = 0;
    graphs::VertexIndex state_vertex_index = 0;

    StateGraphVertexRef() = default;
    constexpr StateGraphVertexRef(uint_t state_graph_index_, graphs::VertexIndex state_vertex_index_) noexcept :
        state_graph_index(state_graph_index_),
        state_vertex_index(state_vertex_index_)
    {
    }

    auto cista_members() noexcept { return std::tie(state_graph_index, state_vertex_index); }
    constexpr auto identifying_members() const noexcept { return std::tie(state_graph_index, state_vertex_index); }
};

struct StateGraphEdgeRef : ygg::comparison::Mixin<StateGraphEdgeRef>
{
    uint_t state_graph_index = 0;
    graphs::EdgeIndex state_edge_index = 0;

    StateGraphEdgeRef() = default;
    constexpr StateGraphEdgeRef(uint_t state_graph_index_, graphs::EdgeIndex state_edge_index_) noexcept :
        state_graph_index(state_graph_index_),
        state_edge_index(state_edge_index_)
    {
    }

    auto cista_members() noexcept { return std::tie(state_graph_index, state_edge_index); }
    constexpr auto identifying_members() const noexcept { return std::tie(state_graph_index, state_edge_index); }
};

template<tyr::planning::TaskKind Kind>
struct StateGraphVertexCandidate : ygg::comparison::Mixin<StateGraphVertexCandidate<Kind>>
{
    uint_t state_graph_index = 0;
    tyr::planning::StateView<Kind> state;

    StateGraphVertexCandidate(uint_t state_graph_index_, tyr::planning::StateView<Kind> state_) noexcept :
        state_graph_index(state_graph_index_),
        state(std::move(state_))
    {
    }

    auto cista_members() noexcept { return std::tie(state_graph_index, state); }
    auto identifying_members() const noexcept { return std::tie(state_graph_index, state); }
};

template<tyr::planning::TaskKind Kind>
struct StateGraphTransitionCandidate : ygg::comparison::Mixin<StateGraphTransitionCandidate<Kind>>
{
    uint_t state_graph_index = 0;
    tyr::planning::StateView<Kind> source_state;
    tyr::planning::StateView<Kind> target_state;

    StateGraphTransitionCandidate(uint_t state_graph_index_,
                                  tyr::planning::StateView<Kind> source_state_,
                                  tyr::planning::StateView<Kind> target_state_) noexcept :
        state_graph_index(state_graph_index_),
        source_state(std::move(source_state_)),
        target_state(std::move(target_state_))
    {
    }

    auto cista_members() noexcept { return std::tie(state_graph_index, source_state, target_state); }
    auto identifying_members() const noexcept { return std::tie(state_graph_index, source_state, target_state); }
};

template<typename Policy, typename Kind>
concept IsEquivalencePolicy =
    requires(Policy& policy, StateGraphVertexCandidate<Kind> vertex_candidate, StateGraphTransitionCandidate<Kind> transition_candidate) {
        requires tyr::planning::TaskKind<Kind>;
        { policy.try_insert(vertex_candidate) } -> std::same_as<bool>;
        { policy.try_insert(transition_candidate) } -> std::same_as<bool>;
        { policy.get_or_create_representative(vertex_candidate, StateGraphVertexRef {}) } -> std::same_as<StateGraphVertexRef>;
    };

}  // namespace runir::datasets

#endif
