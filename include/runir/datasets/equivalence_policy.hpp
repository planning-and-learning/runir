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

#include "runir/graphs/declarations.hpp"

#include <concepts>
#include <functional>
#include <tuple>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::datasets
{

template<typename Tag>
struct EquivalencePolicy;

struct StateGraphVertexRef : ygg::comparison::Mixin<StateGraphVertexRef>
{
    ygg::uint_t state_graph_index = 0;
    graphs::VertexIndex state_vertex_index = 0;

    StateGraphVertexRef() = default;
    constexpr StateGraphVertexRef(ygg::uint_t state_graph_index_, graphs::VertexIndex state_vertex_index_) noexcept :
        state_graph_index(state_graph_index_),
        state_vertex_index(state_vertex_index_)
    {
    }

    auto cista_members() noexcept { return std::tie(state_graph_index, state_vertex_index); }
    constexpr auto identifying_members() const noexcept { return std::tie(state_graph_index, state_vertex_index); }
};

template<tyr::TaskKind Kind>
struct StateGraphVertexCandidate : ygg::comparison::Mixin<StateGraphVertexCandidate<Kind>>
{
    ygg::uint_t state_graph_index = 0;
    tyr::planning::StateView<Kind> state;

    StateGraphVertexCandidate(ygg::uint_t state_graph_index_, tyr::planning::StateView<Kind> state_) noexcept :
        state_graph_index(state_graph_index_),
        state(std::move(state_))
    {
    }

    auto cista_members() noexcept { return std::tie(state_graph_index, state); }
    auto identifying_members() const noexcept { return std::tie(state_graph_index, state); }
};

struct RepresentativeResult
{
    StateGraphVertexRef representative;
    bool inserted;
};

/// Called at most once, and only when the policy creates a new equivalence class.
using StateGraphVertexFactory = std::function<StateGraphVertexRef()>;

template<typename Policy, typename Kind>
concept IsEquivalencePolicy = requires(Policy& policy, StateGraphVertexCandidate<Kind> vertex_candidate, const StateGraphVertexFactory& allocate) {
    requires tyr::TaskKind<Kind>;
    { policy.get_or_create_representative(vertex_candidate, allocate) } -> std::same_as<RepresentativeResult>;
};

}  // namespace runir::datasets

#endif
