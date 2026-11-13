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

#include "runir/datasets/graph_types.hpp"

#include <concepts>
#include <tuple>
#include <tyr/planning/state_repository.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>

namespace runir::datasets
{

template<typename Tag>
struct EquivalencePolicy;

struct StateGraphVertexRef : urd::compared_by_identity<StateGraphVertexRef>
{
    ygg::uint_t state_graph_index = 0;
    VertexIndex state_vertex_index = 0;

    StateGraphVertexRef() = default;
    constexpr StateGraphVertexRef(ygg::uint_t state_graph_index_, VertexIndex state_vertex_index_) noexcept :
        state_graph_index(state_graph_index_),
        state_vertex_index(state_vertex_index_)
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&StateGraphVertexRef::state_graph_index, &StateGraphVertexRef::state_vertex_index> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&StateGraphVertexRef::state_graph_index, &StateGraphVertexRef::state_vertex_index> {}; }
};

template<tyr::TaskKind Kind>
struct StateGraphVertexCandidate : urd::compared_by_identity<StateGraphVertexCandidate<Kind>>
{
    ygg::uint_t state_graph_index = 0;
    tyr::planning::StateView<Kind> state;

    StateGraphVertexCandidate(ygg::uint_t state_graph_index_, tyr::planning::StateView<Kind> state_) noexcept :
        state_graph_index(state_graph_index_),
        state(std::move(state_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&StateGraphVertexCandidate::state_graph_index, &StateGraphVertexCandidate::state> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&StateGraphVertexCandidate::state_graph_index, &StateGraphVertexCandidate::state> {}; }
};

/// A policy names the representative of a state's equivalence class: get_or_create_representative(candidate, allocate)
/// returns the class's representative, calling allocate() (once) for it only where the class is new.
template<typename Policy, typename Kind>
concept IsEquivalencePolicy = tyr::TaskKind<Kind> && requires(Policy& policy, StateGraphVertexCandidate<Kind> candidate) {
    { policy.get_or_create_representative(candidate, [] { return StateGraphVertexRef {}; }) } -> std::same_as<StateGraphVertexRef>;
};

}  // namespace runir::datasets

#endif
