/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "runir/kr/database/semantics/distance.hpp"

#include <ranges>
#include <span>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::database::detail
{
VertexSet::handle_type DistanceGraph::insert_vertex(std::span<const ygg::Value> tuple)
{
    // Reserve the maximum distance as infinity; a simple path has fewer edges than vertices.
    if (vertices.size() >= distance_infinity)
    {
        if (const auto existing = vertices.find(tuple); !existing.is_none())
            return existing;
        throw std::length_error("Distance: vertex count exceeds the finite distance range.");
    }
    return vertices.insert(tuple).first;
}
}

namespace runir::kr::database::detail
{
std::pair<VertexSet::handle_type, VertexSet::handle_type> DistanceGraph::insert_endpoints(std::span<const ygg::Value> edge)
{
    const auto from = insert_vertex(edge.first(tuple_size));
    return { from, insert_vertex(edge.subspan(tuple_size, tuple_size)) };
}
}

namespace runir::kr::database::detail
{
bool DistanceGraph::contains_edge(VertexSet::handle_type from, VertexSet::handle_type to) const
{
    for (const auto [first, last] = outgoing.equal_range(from); const auto target : std::ranges::subrange(first, last) | std::views::values)
        if (target == to)
            return true;
    return false;
}
}

namespace runir::kr::database::detail
{
void DistanceGraph::clear()
{
    outgoing.clear();
    vertices.clear();
}
}

namespace runir::kr::database::detail
{
void breadth_first_search(const DistanceGraph& graph,
                          VertexSet::handle_type root,
                          urd::Store<urd::sequence<ygg::uint_t, VertexSet::handle_type>>& distances,
                          urd::Store<urd::sequence<VertexSet::handle_type>>& queue)
{
    distances.assign(graph.vertices.size(), distance_infinity);
    queue.clear();
    distances[root] = 0;
    queue.push_back(root);
    for (size_t position = 0; position < queue.size(); ++position)
    {
        const auto from = queue[position];
        for (const auto [first, last] = graph.outgoing.equal_range(from); const auto to : std::ranges::subrange(first, last) | std::views::values)
            if (distances[to] == distance_infinity)
            {
                distances[to] = distances[from] + 1;
                queue.push_back(to);
            }
    }
}
}

namespace runir::kr::database::detail
{
std::span<const ygg::Value> make_distance_row(const DistanceGraph& graph,
                                              VertexSet::handle_type from,
                                              VertexSet::handle_type to,
                                              ygg::uint_t length,
                                              urd::Store<urd::sequence<ygg::Value>>& row)
{
    assert(length != distance_infinity);
    row.clear();
    const auto source = graph.vertices.at(from);
    const auto target = graph.vertices.at(to);
    row.insert(row.end(), source.begin(), source.end());
    row.insert(row.end(), target.begin(), target.end());
    row.push_back(ygg::encode_value(length));
    return row;
}
}

namespace runir::kr::database
{
urd::data<DistancePlan> make_distance_plan(std::span<const urd::data<ygg::ColumnLayout>> sources,
                                           std::span<const urd::data<ygg::ColumnLayout>> edges,
                                           std::span<const urd::data<ygg::ColumnLayout>> targets,
                                           urd::index<ygg::Column> distance_column)
{
    for (const auto columns : { sources, edges, targets })
        ygg::validate_columns(columns);
    auto plan = urd::data<DistancePlan> {};
    plan.source_columns.set(sources.begin(), sources.end());
    plan.edge_columns.set(edges.begin(), edges.end());
    plan.target_columns.set(targets.begin(), targets.end());
    plan.output_columns.set(edges.begin(), edges.end());
    const auto k = arity(plan);
    if (plan.target_columns.size() != k || plan.edge_columns.size() % 2 != 0 || plan.edge_columns.size() / 2 != k)
        throw std::invalid_argument("Distance: input arities must be k, 2k, k.");
    for (size_t i = 0; i < k; ++i)
        if (plan.source_columns[i].type != plan.target_columns[i].type || plan.source_columns[i].type != plan.edge_columns[i].type
            || plan.source_columns[i].type != plan.edge_columns[k + i].type)
            throw std::invalid_argument("Distance: vertex field types must agree by position.");
    plan.output_columns.push_back(urd::data<ygg::ColumnLayout> { distance_column, ygg::type_id<ygg::uint_t> });
    ygg::validate_columns(plan.output_columns);
    return plan;
}
}

namespace runir::kr::database
{
DistanceWorkspace::DistanceWorkspace(const urd::data<DistancePlan>& plan) : m_graph(tuple_size(plan)) {}
}

namespace runir::kr::database
{
bool DistanceWorkspace::matches(const urd::data<DistancePlan>& plan) const noexcept { return m_graph.tuple_size == tuple_size(plan); }
}

namespace runir::kr::database
{
size_t DistanceWorkspace::memory_usage() const { return urd::memory_usage(*this); }
}
