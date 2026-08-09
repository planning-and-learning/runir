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

#include "runir/datasets/object_graph_color.hpp"
#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"

#include <cstddef>
#include <memory>
#include <tuple>
#include <tyr/planning/ground/state_view.hpp>
#include <tyr/planning/lifted/state_view.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::datasets
{

using ObjectGraphBuilder = graphs::StaticGraphBuilder<ColorView, std::tuple<>>;

using ObjectGraph = graphs::StaticGraph<ColorView, std::tuple<>>;

inline auto get_vertex_color(graphs::Vertex<ObjectGraph, ColorView> vertex) -> ygg::Index<Color> { return vertex.get_property().get_index(); }

template<tyr::TaskKind Kind>
auto create_object_graph(tyr::planning::StateView<Kind> state, ColorRepository& repository) -> std::unique_ptr<ObjectGraph>;

}  // namespace runir::datasets

#endif
