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

#ifndef RUNIR_DATASETS_OBJECT_GRAPH_HPP_
#define RUNIR_DATASETS_OBJECT_GRAPH_HPP_

#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"

#include <cstddef>
#include <memory>
#include <tuple>
#include <tyr/planning/planning.hpp>
#include <variant>
#include <vector>

namespace runir::datasets
{

enum class ObjectGraphPredicateKind
{
    STATE_PREDICATE,
    GOAL_PREDICATE
};

using ObjectGraphPredicateVariant = std::variant<tyr::formalism::planning::PredicateView<tyr::formalism::StaticTag>,
                                                 tyr::formalism::planning::PredicateView<tyr::formalism::FluentTag>>;

struct ObjectGraphVertexLabelEntry
{
    ObjectGraphPredicateVariant predicate;
    std::size_t argument_position = 0;
    ObjectGraphPredicateKind kind = ObjectGraphPredicateKind::STATE_PREDICATE;

    auto identifying_members() const noexcept { return std::tie(predicate, argument_position, kind); }
};

struct ObjectGraphVertexLabel
{
    std::vector<ObjectGraphVertexLabelEntry> labels;

    auto identifying_members() const noexcept { return std::tie(labels); }
};

template<tyr::planning::TaskKind Kind>
using ObjectGraphBuilder = graphs::StaticGraphBuilder<ObjectGraphVertexLabel, std::tuple<>>;

template<tyr::planning::TaskKind Kind>
using ObjectGraph = graphs::StaticGraph<ObjectGraphVertexLabel, std::tuple<>>;

template<tyr::planning::TaskKind Kind>
auto create_object_graph(tyr::planning::StateView<Kind> state) -> std::unique_ptr<ObjectGraph<Kind>>;

}  // namespace runir::datasets

#endif
