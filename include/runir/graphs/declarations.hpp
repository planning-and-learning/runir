#ifndef RUNIR_GRAPHS_DECLARATIONS_HPP_
#define RUNIR_GRAPHS_DECLARATIONS_HPP_

#include <concepts>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <urd/elements/equal_to.hpp>
#include <urd/elements/hash.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <urd/elements/flat.hpp>

namespace runir::graphs
{

using VertexIndex = ygg::uint_t;
using VertexIndexList = urd::Store<urd::sequence<VertexIndex>>;
using VertexIndexSet = urd::Store<urd::set<VertexIndex>>;

using EdgeIndex = ygg::uint_t;
using EdgeIndexList = urd::Store<urd::sequence<EdgeIndex>>;
using EdgeIndexSet = urd::Store<urd::set<EdgeIndex>>;

using Degree = ygg::uint_t;
using DegreeList = urd::Store<urd::sequence<Degree>>;

struct DenseIndexRangeTag;
struct SparseIndexRangeTag;

template<typename T>
concept Property = requires(const T& lhs, const T& rhs) {
    { urd::Hash<T> {}(lhs) } -> std::convertible_to<std::size_t>;
    { urd::EqualTo<T> {}(lhs, rhs) } -> std::same_as<bool>;
};

template<Property VertexProperty, Property EdgeProperty>
class StaticGraphBuilder;

template<Property VertexProperty, Property EdgeProperty>
class StaticGraph;

template<Property VertexProperty, Property EdgeProperty>
class BidirectionalStaticGraph;

template<Property VertexProperty, Property EdgeProperty>
class DynamicGraph;

template<typename G>
struct GraphTraits
{
    using VertexIndexRange = SparseIndexRangeTag;
    using EdgeIndexRange = SparseIndexRangeTag;
    using OutEdgeIndexRange = SparseIndexRangeTag;
};

template<Property VertexProperty, Property EdgeProperty>
struct GraphTraits<StaticGraphBuilder<VertexProperty, EdgeProperty>>
{
    using VertexIndexRange = DenseIndexRangeTag;
    using EdgeIndexRange = DenseIndexRangeTag;
    using OutEdgeIndexRange = SparseIndexRangeTag;
};

template<Property VertexProperty, Property EdgeProperty>
struct GraphTraits<StaticGraph<VertexProperty, EdgeProperty>>
{
    using VertexIndexRange = DenseIndexRangeTag;
    using EdgeIndexRange = DenseIndexRangeTag;
    using OutEdgeIndexRange = DenseIndexRangeTag;
};

template<typename G, Property P = std::tuple<>>
class Vertex;

template<typename G, Property P = std::tuple<>>
class Edge;

template<Property P>
struct VertexProperty;

template<Property P>
struct EdgeProperty;

/// A property's handle in its graph's property pool (VertexPropertyMap, EdgePropertyMap).
template<Property P>
using VertexPropertyIndex = urd::index<urd::data<VertexProperty<P>>>;

template<Property P>
using EdgePropertyIndex = urd::index<urd::data<EdgeProperty<P>>>;

template<typename G, typename VP, typename EP>
concept IsGraphWithProperties = requires(const G& graph, VertexPropertyIndex<VP> vertex_property, EdgePropertyIndex<EP> edge_property) {
    { graph.get_vertex_property(vertex_property) } -> std::same_as<const VP&>;
    { graph.get_edge_property(edge_property) } -> std::same_as<const EP&>;
};

template<typename G>
using GraphVertexPropertyType = typename G::VertexPropertyType;

template<typename G>
using GraphEdgePropertyType = typename G::EdgePropertyType;

template<typename G>
concept IsGraph =
    IsGraphWithProperties<G, GraphVertexPropertyType<G>, GraphEdgePropertyType<G>> && requires(const G& graph, VertexIndex vertex, EdgeIndex edge) {
        { graph.get_num_vertices() } -> std::convertible_to<std::size_t>;
        { graph.get_num_edges() } -> std::convertible_to<std::size_t>;
        { graph.get_source(edge) } -> std::same_as<VertexIndex>;
        { graph.get_target(edge) } -> std::same_as<VertexIndex>;
        { graph.get_out_degree(vertex) } -> std::convertible_to<Degree>;

        // The Boost adapter returns these iterators after the range wrapper is destroyed.
        requires std::same_as<std::ranges::range_value_t<decltype(graph.get_vertex_indices())>, VertexIndex>;
        { graph.get_vertex_indices() } -> std::ranges::forward_range;
        { graph.get_vertex_indices() } -> std::ranges::common_range;
        { graph.get_vertex_indices() } -> std::ranges::borrowed_range;
        requires std::same_as<std::ranges::range_value_t<decltype(graph.get_edge_indices())>, EdgeIndex>;
        { graph.get_edge_indices() } -> std::ranges::forward_range;
        { graph.get_edge_indices() } -> std::ranges::common_range;
        { graph.get_edge_indices() } -> std::ranges::borrowed_range;
        requires std::same_as<std::ranges::range_value_t<decltype(graph.get_out_edge_indices(vertex))>, EdgeIndex>;
        { graph.get_out_edge_indices(vertex) } -> std::ranges::forward_range;
        { graph.get_out_edge_indices(vertex) } -> std::ranges::common_range;
        { graph.get_out_edge_indices(vertex) } -> std::ranges::borrowed_range;
    };

template<typename G>
concept IsDenseGraph = IsGraph<G> && std::same_as<typename GraphTraits<std::remove_cvref_t<G>>::VertexIndexRange, DenseIndexRangeTag>
                       && std::same_as<typename GraphTraits<std::remove_cvref_t<G>>::EdgeIndexRange, DenseIndexRangeTag>
                       && std::same_as<typename GraphTraits<std::remove_cvref_t<G>>::OutEdgeIndexRange, DenseIndexRangeTag>;

template<IsDenseGraph G>
class BackwardStaticGraphView;

template<IsDenseGraph G>
struct GraphTraits<BackwardStaticGraphView<G>>
{
    using VertexIndexRange = DenseIndexRangeTag;
    using EdgeIndexRange = DenseIndexRangeTag;
    using OutEdgeIndexRange = DenseIndexRangeTag;
};

template<typename T>
concept IsVertex = requires(T vertex) {
    { vertex.get_index() } -> std::convertible_to<VertexIndex>;
    { vertex.get_property_index() } -> std::same_as<typename T::PropertyIndexType>;
    { vertex.get_property() } -> std::same_as<const typename T::PropertyType&>;
};

template<typename T>
concept IsEdge = requires(T edge) {
    { edge.get_index() } -> std::convertible_to<EdgeIndex>;
    { edge.get_source() } -> std::convertible_to<VertexIndex>;
    { edge.get_target() } -> std::convertible_to<VertexIndex>;
    { edge.get_property_index() } -> std::same_as<typename T::PropertyIndexType>;
    { edge.get_property() } -> std::same_as<const typename T::PropertyType&>;
};

}  // namespace runir::graphs

#endif
