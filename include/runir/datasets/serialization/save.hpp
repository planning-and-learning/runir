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

#ifndef RUNIR_DATASETS_SERIALIZATION_SAVE_HPP_
#define RUNIR_DATASETS_SERIALIZATION_SAVE_HPP_

#include "runir/datasets/serialization/context.hpp"
#include "runir/datasets/serialization/converters.hpp"
#include "runir/datasets/state_graph.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace runir::datasets::serialization
{

namespace detail
{

inline auto read_task_file(const std::filesystem::path& task_filepath) -> std::string
{
    auto file = std::ifstream(task_filepath);
    if (!file)
        throw std::runtime_error("Could not open task file: " + task_filepath.string() + ".");

    auto stream = std::ostringstream {};
    stream << file.rdbuf();
    return stream.str();
}

template<typename Table, typename Range, typename Transform>
auto indexed(const Table& table, Range&& range, Transform&& transform)
{
    auto result = std::vector<SymbolIndex> {};
    for (const auto value : range)
        result.push_back(table.index(transform(value)));
    std::sort(result.begin(), result.end());
    return result;
}

template<tyr::planning::TaskKind Kind>
void collect(StateSerializationContext context, tyr::planning::StateView<Kind> state)
{
    for (const auto atom : state.get_static_atoms_view())
        context.symbols->static_atoms.insert(archive(atom));
    for (const auto fact : state.get_fluent_facts_view())
        context.symbols->fluent_facts.insert(archive(fact));
    for (const auto atom : state.get_derived_atoms_view())
        context.symbols->derived_atoms.insert(archive(atom));
    for (const auto& value : state.get_static_fterm_values_view())
        context.symbols->static_numeric_values.insert(archive(value));
    for (const auto& value : state.get_fluent_fterm_values_view())
        context.symbols->fluent_numeric_values.insert(archive(value));
}

inline void collect(EdgeSerializationContext context, const StateGraphEdgeLabel& label) { context.symbols->ground_actions.insert(archive(label)); }

template<tyr::planning::TaskKind Kind>
auto save(StateSerializationContext context, tyr::planning::StateView<Kind> state) -> StateArchive
{
    return StateArchive {
        indexed(context.symbols->static_atoms, state.get_static_atoms_view(), [](auto atom) { return archive(atom); }),
        indexed(context.symbols->fluent_facts, state.get_fluent_facts_view(), [](auto fact) { return archive(fact); }),
        indexed(context.symbols->derived_atoms, state.get_derived_atoms_view(), [](auto atom) { return archive(atom); }),
        indexed(context.symbols->static_numeric_values, state.get_static_fterm_values_view(), [](const auto& value) { return archive(value); }),
        indexed(context.symbols->fluent_numeric_values, state.get_fluent_fterm_values_view(), [](const auto& value) { return archive(value); })
    };
}

template<tyr::planning::TaskKind Kind>
auto save(StateSerializationContext context, const StateGraphVertexLabel<Kind>& label) -> StateArchive
{
    return save(context, label.state);
}

template<tyr::planning::TaskKind Kind>
auto save(StateSerializationContext context, const AnnotatedStateGraphVertexLabel<Kind>& label) -> AnnotatedStateArchive
{
    return AnnotatedStateArchive { save(context, label.state), label.goal_distance, label.is_initial, label.is_goal, label.is_alive, label.is_unsolvable };
}

inline auto save(EdgeSerializationContext context, const StateGraphEdgeLabel& label)
{
    return std::pair { context.symbols->ground_actions.index(archive(label)), label.cost };
}

template<typename Graph>
void collect(GraphSerializationContext& context, const Graph& graph)
{
    const auto& forward_graph = graph.get_forward_graph();

    for (const auto vertex : forward_graph.get_vertex_indices())
        collect(context.state_context(), forward_graph.get_vertex(vertex).get_property().state);
    for (const auto edge : forward_graph.get_edge_indices())
        collect(context.edge_context(), forward_graph.get_edge(edge).get_property());

    context.symbols.finalize();
}

template<typename State, typename Graph>
auto save_graph(GraphSerializationContext& context, const Graph& graph)
{
    const auto& forward_graph = graph.get_forward_graph();

    auto states = std::vector<State> {};
    states.reserve(forward_graph.get_num_vertices());
    for (const auto vertex : forward_graph.get_vertex_indices())
        states.push_back(save(context.state_context(), forward_graph.get_vertex(vertex).get_property()));

    auto edges = std::vector<EdgeArchive> {};
    edges.reserve(forward_graph.get_num_edges());
    for (const auto edge : forward_graph.get_edge_indices())
    {
        const auto [action, cost] = save(context.edge_context(), forward_graph.get_edge(edge).get_property());
        edges.push_back(EdgeArchive { forward_graph.get_source(edge), forward_graph.get_target(edge), action, cost });
    }

    return std::pair { std::move(states), std::move(edges) };
}

}  // namespace detail

template<tyr::planning::TaskKind Kind>
auto save(const std::filesystem::path& task_filepath, const StateGraph<Kind>& graph) -> StateGraphArchive
{
    auto context = GraphSerializationContext {};
    detail::collect(context, graph);
    auto [states, edges] = detail::save_graph<StateArchive>(context, graph);
    return StateGraphArchive { detail::read_task_file(task_filepath), context.symbols.release(), std::move(states), std::move(edges) };
}

template<tyr::planning::TaskKind Kind>
auto save(const std::filesystem::path& task_filepath, const AnnotatedStateGraph<Kind>& graph) -> AnnotatedStateGraphArchive
{
    auto context = GraphSerializationContext {};
    detail::collect(context, graph);
    auto [states, edges] = detail::save_graph<AnnotatedStateArchive>(context, graph);
    return AnnotatedStateGraphArchive { detail::read_task_file(task_filepath), context.symbols.release(), std::move(states), std::move(edges) };
}

}  // namespace runir::datasets::serialization

#endif
