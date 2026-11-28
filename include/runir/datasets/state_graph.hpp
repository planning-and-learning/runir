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

#ifndef RUNIR_DATASETS_STATE_GRAPH_HPP_
#define RUNIR_DATASETS_STATE_GRAPH_HPP_

#include "runir/datasets/config.hpp"
#include "runir/datasets/declarations.hpp"
#include "runir/datasets/graph_types.hpp"
#include "runir/datasets/task_class.hpp"

#include <chrono>
#include <limits>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/formalism/repository.hpp>
#include <tyr/planning/algorithms/declarations.hpp>
#include <urd/elements/concepts.hpp>
#include <urd/repr/graphs/csr_graph.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/formalism/row_view.hpp>

namespace urd
{
template<tyr::TaskKind Kind>
struct data<runir::datasets::StateGraphVertexLabel<Kind>>
{
    tyr::planning::StateView<Kind> state;

    data() = default;

    data(tyr::planning::StateView<Kind> state_) noexcept : state(state_) {}

    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"state", &data::state>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::state> {}; }
};

template<tyr::TaskKind Kind>
struct data<runir::datasets::AnnotatedStateGraphVertexLabel<Kind>>
{
    tyr::planning::StateView<Kind> state;
    ygg::float_t goal_distance {};
    bool is_initial {};
    bool is_goal {};
    bool is_alive {};
    bool is_unsolvable {};

    data() = default;

    data(tyr::planning::StateView<Kind> state_, ygg::float_t goal_distance_, bool is_initial_, bool is_goal_, bool is_alive_, bool is_unsolvable_) noexcept :
        state(state_),
        goal_distance(goal_distance_),
        is_initial(is_initial_),
        is_goal(is_goal_),
        is_alive(is_alive_),
        is_unsolvable(is_unsolvable_)
    {
    }

    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"state", &data::state>,
                           urd::field<"goal_distance", &data::goal_distance>,
                           urd::field<"is_initial", &data::is_initial>,
                           urd::field<"is_goal", &data::is_goal>,
                           urd::field<"is_alive", &data::is_alive>,
                           urd::field<"is_unsolvable", &data::is_unsolvable>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::state, &data::goal_distance, &data::is_initial, &data::is_goal, &data::is_alive, &data::is_unsolvable> {}; }
};

template<>
struct data<runir::datasets::StateGraphEdgeLabel>
{
    tyr::formalism::ActionRowView action;
    ygg::float_t cost {};

    data() = default;

    data(tyr::formalism::ActionRowView action_, ygg::float_t cost_) noexcept : action(action_), cost(cost_) {}

    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"action", &data::action>, urd::field<"cost", &data::cost>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::action, &data::cost> {}; }
};
}  // namespace urd

namespace runir::datasets
{

/// A task's reachable states and their transitions: a frozen graph that keeps its in-edges (goal distances search it
/// backwards), each state labelled by its state, each edge by its action and cost.
template<tyr::TaskKind Kind>
using StateGraph = urd::Store<urd::graph<VertexIndex, urd::data<StateGraphEdgeLabel>, urd::data<StateGraphVertexLabel<Kind>>>, urd::frozen, urd::bidirectional>;

/// A state graph whose states know their goal distance and whether they are initial, goal, alive or unsolvable.
template<tyr::TaskKind Kind>
using AnnotatedStateGraph =
    urd::Store<urd::graph<VertexIndex, urd::data<StateGraphEdgeLabel>, urd::data<AnnotatedStateGraphVertexLabel<Kind>>>, urd::frozen, urd::bidirectional>;
// The concepts it is bound and searched by (urd/graphs.hpp): a renamed member is a compile error, not a lost feature.
static_assert(urd::bidirectional_graph<StateGraph<tyr::GroundTag>> && urd::edge_list_graph<StateGraph<tyr::GroundTag>>);
static_assert(urd::bidirectional_graph<AnnotatedStateGraph<tyr::LiftedTag>> && urd::edge_list_graph<AnnotatedStateGraph<tyr::LiftedTag>>);

struct StateGraphGenerationOptions
{
    ygg::uint_t max_num_states = std::numeric_limits<ygg::uint_t>::max();
    std::optional<std::chrono::steady_clock::duration> max_time = std::nullopt;
};

template<tyr::TaskKind Kind>
struct StateGraphGenerationResult
{
    std::unique_ptr<StateGraph<Kind>> graph;
    tyr::planning::SearchStatus status = tyr::planning::SearchStatus::IN_PROGRESS;
};

template<tyr::TaskKind Kind>
auto generate_state_graph_result(TaskSearchContext<Kind>& context, const StateGraphGenerationOptions& options = StateGraphGenerationOptions())
    -> StateGraphGenerationResult<Kind>;

template<tyr::TaskKind Kind>
auto generate_state_graph(TaskSearchContext<Kind>& context, const StateGraphGenerationOptions& options = StateGraphGenerationOptions())
    -> std::unique_ptr<StateGraph<Kind>>;

template<tyr::TaskKind Kind>
auto annotate_state_graph(TaskSearchContext<Kind>& context, const StateGraph<Kind>& graph, StateGraphCostMode cost_mode)
    -> std::unique_ptr<AnnotatedStateGraph<Kind>>;

}  // namespace runir::datasets

#endif
