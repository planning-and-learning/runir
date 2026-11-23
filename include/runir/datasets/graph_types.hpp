#ifndef RUNIR_DATASETS_GRAPH_TYPES_HPP_
#define RUNIR_DATASETS_GRAPH_TYPES_HPP_

// The vertices and edges of runir's graphs: urd graphs (urd::graph, urd/graphs.hpp) over these vertex indices, labels
// inside, whose edges keep the numbers they were given. The algorithms on them are vefr's.

#include <cstddef>
#include <ranges>
#include <type_traits>
#include <urd/elements/requirements.hpp>
#include <urd/graphs.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
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

namespace ygg::serialization
{
/// A runir graph is serialized as its vertices' labels by vertex, its edges as (source, target) by edge number, and the
/// edges' labels (pyyggdrasil.serialization). It is declared here, beside the concept, because a graph is a urd Store:
/// its namespace is urd's, so the archive's namespace finds it.
template<typename Archive, ::runir::labeled_graph G>
void describe_fields(Archive& ar, std::type_identity<G> /*type*/)
{
    ar.field("vertices",
             [](const G& value) { return value.vertices() | std::views::transform([&value](auto v) -> decltype(auto) { return value.vertex_value(v); }); });
    ar.field("edges",
             [](const G& value)
             {
                 auto result = std::vector<std::pair<::runir::VertexIndex, ::runir::VertexIndex>>();
                 for (const auto e : value.edges())
                     result.emplace_back(value.source(e), value.target(e));
                 return result;
             });
    ar.field("edge_properties",
             [](const G& value) { return value.edges() | std::views::transform([&value](std::size_t e) -> decltype(auto) { return value.edge_value(e); }); });
}
}  // namespace ygg::serialization

#endif
