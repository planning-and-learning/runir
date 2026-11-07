#ifndef RUNIR_GRAPHS_DECLARATIONS_HPP_
#define RUNIR_GRAPHS_DECLARATIONS_HPP_

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <urd/elements/equal_to.hpp>
#include <urd/elements/hash.hpp>
#include <urd/repr/maps/unordered_flat_map.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/values/indexed.hpp>
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

namespace detail
{
/// A graph's member function F applied to what a range of the graph gives (an edge index: its edge, its target): the
/// small function object its ranges are read through (urd::transform), holding the graph.
template<typename G, auto F>
struct ReadThrough
{
    const G* graph = nullptr;

    template<typename X>
    [[nodiscard]] decltype(auto) operator()(X x) const
    { return (graph->*F)(x); }
};
}  // namespace detail

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

template<Property P = std::tuple<>>
class Vertex;

template<Property P = std::tuple<>>
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
concept IsGraphWithProperties = requires(const G& graph, VertexIndex vertex, EdgeIndex edge) {
    { graph.get_vertex_property(vertex) } -> std::same_as<const VP&>;
    { graph.get_edge_property(edge) } -> std::same_as<const EP&>;
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

/// A graph's vertices in increasing order, each numbered by its position: vertices.position(vertices.find(vertex)) is
/// its dense index, vertices[i] the vertex numbered i. Algorithms that number vertices 0..n-1 take them from here.
using DenseVertexIndices = urd::Store<urd::set<VertexIndex>, urd::positional>;

template<IsGraph G>
auto dense_vertex_indices(const G& graph) -> DenseVertexIndices
{
    auto sorted = VertexIndexList();
    sorted.reserve(graph.get_num_vertices());
    for (auto vertex : graph.get_vertex_indices())
        sorted.push_back(vertex);
    std::sort(sorted.begin(), sorted.end());

    auto result = DenseVertexIndices(urd::layer { 0 });
    for (auto vertex : sorted)
        result.insert(vertex);
    return result;
}

}  // namespace runir::graphs

#endif
