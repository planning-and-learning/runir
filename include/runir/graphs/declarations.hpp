#ifndef RUNIR_GRAPHS_DECLARATIONS_HPP_
#define RUNIR_GRAPHS_DECLARATIONS_HPP_

#include "runir/common/config.hpp"

#include <concepts>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <tyr/common/declarations.hpp>
#include <tyr/common/equal_to.hpp>
#include <tyr/common/hash.hpp>
#include <tyr/common/types.hpp>
#include <unordered_set>
#include <vector>

namespace runir::graphs
{

using VertexIndex = uint_t;
using VertexIndexList = std::vector<VertexIndex>;
using VertexIndexSet = std::unordered_set<VertexIndex>;

using EdgeIndex = uint_t;
using EdgeIndexList = std::vector<EdgeIndex>;
using EdgeIndexSet = std::unordered_set<EdgeIndex>;

using Degree = uint_t;
using DegreeList = std::vector<Degree>;

struct DenseIndexRangeTag;
struct SparseIndexRangeTag;

template<typename T>
concept Property = requires(const T& lhs, const T& rhs) {
    { tyr::Hash<T> {}(lhs) } -> std::convertible_to<std::size_t>;
    { tyr::EqualTo<T> {}(lhs, rhs) } -> std::same_as<bool>;
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

template<Property P>
using VertexPropertyIndex = tyr::Index<VertexProperty<P>>;

template<Property P>
using EdgePropertyIndex = tyr::Index<EdgeProperty<P>>;

template<typename Tag>
class PropertyMap;

template<typename G, typename VP, typename EP>
concept IsGraphWithProperties = requires(const G& graph, VertexPropertyIndex<VP> vertex_property, EdgePropertyIndex<EP> edge_property) {
    { graph.get_vertex_property(vertex_property) } -> std::same_as<const VP&>;
    { graph.get_edge_property(edge_property) } -> std::same_as<const EP&>;
};

template<typename G>
concept IsGraph = requires {
    typename G::VertexPropertyType;
    typename G::EdgePropertyType;
} && IsGraphWithProperties<G, typename G::VertexPropertyType, typename G::EdgePropertyType>;

template<typename G>
concept IsDenseGraph =
    requires {
        typename GraphTraits<std::remove_cvref_t<G>>::VertexIndexRange;
        typename GraphTraits<std::remove_cvref_t<G>>::EdgeIndexRange;
        typename GraphTraits<std::remove_cvref_t<G>>::OutEdgeIndexRange;
    } && std::same_as<typename GraphTraits<std::remove_cvref_t<G>>::VertexIndexRange, DenseIndexRangeTag>
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
    typename T::PropertyType;
    typename T::PropertyIndexType;

    { vertex.get_index() } -> std::convertible_to<VertexIndex>;
    { vertex.get_property_index() } -> std::same_as<typename T::PropertyIndexType>;
    { vertex.get_property() } -> std::same_as<const typename T::PropertyType&>;
};

template<typename T>
concept IsEdge = requires(T edge) {
    typename T::PropertyType;
    typename T::PropertyIndexType;

    { edge.get_index() } -> std::convertible_to<EdgeIndex>;
    { edge.get_source() } -> std::convertible_to<VertexIndex>;
    { edge.get_target() } -> std::convertible_to<VertexIndex>;
    { edge.get_property_index() } -> std::same_as<typename T::PropertyIndexType>;
    { edge.get_property() } -> std::same_as<const typename T::PropertyType&>;
};

template<typename T>
concept IsStaticGraphBuilder = requires(T graph, VertexIndex vertex, EdgeIndex edge) {
    typename T::VertexType;
    typename T::EdgeType;

    { graph.get_vertices() } -> std::same_as<const std::vector<typename T::VertexType>&>;
    { graph.get_edges() } -> std::same_as<const std::vector<typename T::EdgeType>&>;
    { graph.get_vertex(vertex) } -> std::same_as<const typename T::VertexType&>;
    { graph.get_edge(edge) } -> std::same_as<const typename T::EdgeType&>;
    { graph.get_num_vertices() } -> std::same_as<std::size_t>;
    { graph.get_num_edges() } -> std::same_as<std::size_t>;
};

}  // namespace runir::graphs

#endif
