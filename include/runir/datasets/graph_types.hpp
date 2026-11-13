#ifndef RUNIR_DATASETS_GRAPH_TYPES_HPP_
#define RUNIR_DATASETS_GRAPH_TYPES_HPP_

// The vertices and edges of runir's graphs: urd graphs (urd::graph, urd/graphs.hpp) over these vertex indices, labels
// inside, whose edges keep the numbers they were given. The algorithms on them are vefr's.

#include <cstddef>
#include <urd/elements/requirements.hpp>
#include <urd/graphs.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <yggdrasil/core/config.hpp>

namespace runir
{

using VertexIndex = ygg::uint_t;
using VertexIndexList = urd::Store<urd::sequence<VertexIndex>>;
using EdgeIndex = ygg::uint_t;
using EdgeIndexList = urd::Store<urd::sequence<EdgeIndex>>;

/// The label of a graph's vertices, and of its edges (std::monostate, a unit, for none).
template<typename G>
using vertex_label_t = typename urd::container_traits<G>::vertex_value_type;
template<typename G>
using edge_label_t = typename urd::container_traits<G>::edge_value_type;

/// A runir graph: an urd graph whose vertices carry labels, which are not a unit (the state, equivalence, object, proof
/// and policy graphs).
template<typename G>
concept labeled_graph = urd::graph_like<G> && !urd::unit<vertex_label_t<G>>;

}  // namespace runir

#endif
