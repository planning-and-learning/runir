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

#include "runir/datasets/graph_types.hpp"
#include "runir/datasets/object_graph_color.hpp"

#include <cstddef>
#include <memory>
#include <tuple>
#include <tyr/planning/state_repository.hpp>
#include <urd/repr/graphs/csr_graph.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace runir::datasets
{

/// A state's objects, each colored by the predicates it occurs in, adjacent (both ways) where they occur together in an
/// atom: a simple undirected graph, as vefr::canonical_form reads it, each object labelled by its color.
using ObjectGraph = urd::Store<urd::graph<VertexIndex, std::monostate, ColorView>, urd::frozen, urd::positional>;
// The concepts it is bound and searched by (urd/graphs.hpp): a renamed member is a compile error, not a lost feature.
static_assert(urd::incidence_graph<ObjectGraph> && urd::edge_list_graph<ObjectGraph>);

template<tyr::TaskKind Kind>
auto create_object_graph(tyr::planning::StateView<Kind> state, ColorRepository& repository) -> std::unique_ptr<ObjectGraph>;

}  // namespace runir::datasets

#endif
