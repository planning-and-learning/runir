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

#include "runir/common/config.hpp"
#include "runir/graphs/declarations.hpp"

#include <concepts>
#include <tuple>
#include <tyr/planning/planning.hpp>

namespace runir::datasets
{

struct IdentityEquivalenceTag
{
};

template<typename Tag>
struct EquivalencePolicy;

template<tyr::planning::TaskKind Kind>
struct StateGraphVertexCandidate
{
    uint_t state_graph_index = 0;
    tyr::planning::StateView<Kind> state;

    auto identifying_members() const noexcept { return std::make_tuple(state_graph_index, state.get_index()); }
};

template<tyr::planning::TaskKind Kind>
struct StateGraphTransitionCandidate
{
    uint_t state_graph_index = 0;
    tyr::planning::StateView<Kind> source_state;
    tyr::planning::StateView<Kind> target_state;

    auto identifying_members() const noexcept { return std::make_tuple(state_graph_index, source_state.get_index(), target_state.get_index()); }
};

template<typename Policy, typename Kind>
concept IsEquivalencePolicy =
    requires(Policy& policy, StateGraphVertexCandidate<Kind> vertex_candidate, StateGraphTransitionCandidate<Kind> transition_candidate) {
        requires tyr::planning::TaskKind<Kind>;
        { policy.try_insert(vertex_candidate) } -> std::same_as<bool>;
        { policy.try_insert(transition_candidate) } -> std::same_as<bool>;
    };

template<>
struct EquivalencePolicy<IdentityEquivalenceTag>
{
    template<tyr::planning::TaskKind Kind>
    auto try_insert(const StateGraphVertexCandidate<Kind>& candidate) noexcept -> bool
    {
        static_cast<void>(candidate);
        return true;
    }

    template<tyr::planning::TaskKind Kind>
    auto try_insert(const StateGraphTransitionCandidate<Kind>& candidate) noexcept -> bool
    {
        static_cast<void>(candidate);
        return true;
    }
};

}  // namespace runir::datasets

#endif
