#ifndef RUNIR_GRAPHS_BGL_ALGORITHMS_HPP_
#define RUNIR_GRAPHS_BGL_ALGORITHMS_HPP_

#include "runir/graphs/bgl/graph_adapters.hpp"
#include "runir/graphs/bgl/property_maps.hpp"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>
#include <limits>
#include <queue>
#include <type_traits>

namespace runir::graphs::bgl
{

template<IsGraph G>
class TraversalVisitor
{
public:
    virtual ~TraversalVisitor() = default;

    virtual void initialize_vertex(VertexIndex vertex) { static_cast<void>(vertex); }
    virtual void start_vertex(VertexIndex vertex) { static_cast<void>(vertex); }
    virtual void discover_vertex(VertexIndex vertex) { static_cast<void>(vertex); }
    virtual void examine_vertex(VertexIndex vertex) { static_cast<void>(vertex); }
    virtual void examine_edge(EdgeIndex edge) { static_cast<void>(edge); }
    virtual void tree_edge(EdgeIndex edge) { static_cast<void>(edge); }
    virtual void non_tree_edge(EdgeIndex edge) { static_cast<void>(edge); }
    virtual void gray_target(EdgeIndex edge) { static_cast<void>(edge); }
    virtual void black_target(EdgeIndex edge) { static_cast<void>(edge); }
    virtual void back_edge(EdgeIndex edge) { static_cast<void>(edge); }
    virtual void forward_or_cross_edge(EdgeIndex edge) { static_cast<void>(edge); }
    virtual void finish_vertex(VertexIndex vertex) { static_cast<void>(vertex); }
};

template<IsGraph G>
class BoostTraversalVisitor : public boost::default_bfs_visitor, public boost::default_dfs_visitor
{
private:
    TraversalVisitor<G>* m_visitor;

public:
    explicit BoostTraversalVisitor(TraversalVisitor<G>& visitor) noexcept : m_visitor(&visitor) {}

    void initialize_vertex(VertexIndex vertex, const G&) { m_visitor->initialize_vertex(vertex); }
    void start_vertex(VertexIndex vertex, const G&) { m_visitor->start_vertex(vertex); }
    void discover_vertex(VertexIndex vertex, const G&) { m_visitor->discover_vertex(vertex); }
    void examine_vertex(VertexIndex vertex, const G&) { m_visitor->examine_vertex(vertex); }
    void examine_edge(EdgeIndex edge, const G&) { m_visitor->examine_edge(edge); }
    void tree_edge(EdgeIndex edge, const G&) { m_visitor->tree_edge(edge); }
    void non_tree_edge(EdgeIndex edge, const G&) { m_visitor->non_tree_edge(edge); }
    void gray_target(EdgeIndex edge, const G&) { m_visitor->gray_target(edge); }
    void black_target(EdgeIndex edge, const G&) { m_visitor->black_target(edge); }
    void back_edge(EdgeIndex edge, const G&) { m_visitor->back_edge(edge); }
    void forward_or_cross_edge(EdgeIndex edge, const G&) { m_visitor->forward_or_cross_edge(edge); }
    void finish_vertex(VertexIndex vertex, const G&) { m_visitor->finish_vertex(vertex); }
};

template<typename G>
inline constexpr bool IsDenseGraphV = IsDenseGraph<std::remove_cvref_t<G>>;

template<typename G, typename Value>
using VertexMapStorage = std::conditional_t<IsDenseGraphV<G>, std::vector<Value>, tyr::UnorderedMap<VertexIndex, Value>>;

template<typename G, typename Value>
using EdgeMapStorage = std::conditional_t<IsDenseGraphV<G>, std::vector<Value>, tyr::UnorderedMap<EdgeIndex, Value>>;

template<typename G, typename Value>
using VertexReadWritePropertyMap =
    std::conditional_t<IsDenseGraphV<G>, DenseReadWritePropertyMap<VertexIndex, Value>, SparseReadWritePropertyMap<VertexIndex, Value>>;

template<typename G, typename Value>
using EdgeReadPropertyMap = std::conditional_t<IsDenseGraphV<G>, DenseReadPropertyMap<EdgeIndex, Value>, SparseReadPropertyMap<EdgeIndex, Value>>;

template<typename G>
using VertexIndexPropertyMap = std::conditional_t<IsDenseGraphV<G>, IdentityReadPropertyMap<VertexIndex>, SparseReadPropertyMap<VertexIndex, VertexIndex>>;

template<typename G, typename Value>
using DistanceMatrixStorage =
    std::conditional_t<IsDenseGraphV<G>, std::vector<std::vector<Value>>, tyr::UnorderedMap<VertexIndex, tyr::UnorderedMap<VertexIndex, Value>>>;

template<typename G, typename Value>
using BasicMatrix = std::conditional_t<IsDenseGraphV<G>, DenseBasicMatrix<VertexIndex, Value>, SparseBasicMatrix<VertexIndex, Value>>;

template<IsGraph G>
auto make_vertex_index_map(const G& graph)
{
    if constexpr (IsDenseGraphV<G>)
    {
        static_cast<void>(graph);
        return IdentityReadPropertyMap<VertexIndex>();
    }
    else
    {
        auto remap = tyr::UnorderedMap<VertexIndex, VertexIndex>();
        VertexIndex index = 0;
        for (auto vertex : graph.get_vertex_indices())
            remap.emplace(vertex, index++);
        return remap;
    }
}

template<IsGraph G, typename Value>
auto make_vertex_map_storage(const G& graph, const Value& value)
{
    auto result = VertexMapStorage<G, Value>();

    if constexpr (IsDenseGraphV<G>)
    {
        result.resize(graph.get_num_vertices(), value);
    }
    else
    {
        for (auto vertex : graph.get_vertex_indices())
            result.emplace(vertex, value);
    }

    return result;
}

template<IsGraph G>
auto strong_components(const G& graph)
{
    using Component = typename boost::graph_traits<G>::vertices_size_type;

    auto components = make_vertex_map_storage(graph, Component {});
    auto component_map = VertexReadWritePropertyMap<G, Component>(components);
    auto vertex_index_storage = make_vertex_index_map(graph);

    if constexpr (IsDenseGraphV<G>)
    {
        const auto count = boost::strong_components(graph, component_map, boost::vertex_index_map(vertex_index_storage));
        return std::make_pair(count, components);
    }
    else
    {
        auto vertex_index_map = VertexIndexPropertyMap<G>(vertex_index_storage);
        const auto count = boost::strong_components(graph, component_map, boost::vertex_index_map(vertex_index_map));
        return std::make_pair(count, components);
    }
}

template<IsGraph G, typename SourceInputIterator>
auto breadth_first_visit(const G& graph, SourceInputIterator first, SourceInputIterator last, auto visitor)
{
    using Vertex = typename boost::graph_traits<G>::vertex_descriptor;

    auto colors = make_vertex_map_storage(graph, boost::white_color);
    auto color_map = VertexReadWritePropertyMap<G, boost::default_color_type>(colors);
    auto buffer = boost::queue<Vertex>();

    boost::breadth_first_search(graph, first, last, buffer, visitor, color_map);

    return colors;
}

template<IsGraph G, typename SourceInputIterator>
auto breadth_first_visit(const G& graph, SourceInputIterator first, SourceInputIterator last, TraversalVisitor<G>& visitor)
{
    return breadth_first_visit(graph, first, last, BoostTraversalVisitor<G>(visitor));
}

template<IsGraph G>
auto breadth_first_visit(const G& graph, const std::vector<VertexIndex>& sources, TraversalVisitor<G>& visitor)
{
    return breadth_first_visit(graph, sources.begin(), sources.end(), visitor);
}

template<IsGraph G, typename SourceInputIterator>
auto breadth_first_search(const G& graph, SourceInputIterator first, SourceInputIterator last)
{
    using Vertex = typename boost::graph_traits<G>::vertex_descriptor;
    using Edge = typename boost::graph_traits<G>::edge_descriptor;
    using Distance = uint_t;

    const auto infinity = std::numeric_limits<Distance>::max();
    auto predecessors = make_vertex_map_storage(graph, boost::graph_traits<G>::null_vertex());
    auto distances = make_vertex_map_storage(graph, infinity);
    for (auto it = first; it != last; ++it)
        distances[*it] = 0;

    struct Visitor : boost::bfs_visitor<>
    {
        VertexReadWritePropertyMap<G, Vertex> predecessors;
        VertexReadWritePropertyMap<G, Distance> distances;

        Visitor(VertexReadWritePropertyMap<G, Vertex> predecessors_, VertexReadWritePropertyMap<G, Distance> distances_) :
            predecessors(predecessors_),
            distances(distances_)
        {
        }

        void tree_edge(Edge edge, const G& graph_)
        {
            const auto lhs = source(edge, graph_);
            const auto rhs = target(edge, graph_);
            put(predecessors, rhs, lhs);
            put(distances, rhs, get(distances, lhs) + 1);
        }
    };

    auto predecessor_map = VertexReadWritePropertyMap<G, Vertex>(predecessors);
    auto distance_map = VertexReadWritePropertyMap<G, Distance>(distances);
    auto visitor = Visitor(predecessor_map, distance_map);

    breadth_first_visit(graph, first, last, visitor);

    return std::make_tuple(predecessors, distances);
}

template<IsGraph G>
auto breadth_first_search(const G& graph, const std::vector<VertexIndex>& sources)
{
    return breadth_first_search(graph, sources.begin(), sources.end());
}

template<IsGraph G, typename SourceInputIterator>
auto depth_first_visit(const G& graph, SourceInputIterator first, SourceInputIterator last, auto visitor)
{
    auto colors = make_vertex_map_storage(graph, boost::white_color);
    auto color_map = VertexReadWritePropertyMap<G, boost::default_color_type>(colors);

    for (; first != last; ++first)
        if (get(color_map, *first) == boost::white_color)
            boost::depth_first_search(graph, visitor, color_map, *first);

    return colors;
}

template<IsGraph G, typename SourceInputIterator>
auto depth_first_visit(const G& graph, SourceInputIterator first, SourceInputIterator last, TraversalVisitor<G>& visitor)
{
    return depth_first_visit(graph, first, last, BoostTraversalVisitor<G>(visitor));
}

template<IsGraph G>
auto depth_first_visit(const G& graph, const std::vector<VertexIndex>& sources, TraversalVisitor<G>& visitor)
{
    return depth_first_visit(graph, sources.begin(), sources.end(), visitor);
}

template<IsGraph G, typename SourceInputIterator>
auto depth_first_search(const G& graph, SourceInputIterator first, SourceInputIterator last)
{
    using Vertex = typename boost::graph_traits<G>::vertex_descriptor;
    using Edge = typename boost::graph_traits<G>::edge_descriptor;

    auto predecessors = make_vertex_map_storage(graph, boost::graph_traits<G>::null_vertex());

    struct Visitor : boost::dfs_visitor<>
    {
        VertexReadWritePropertyMap<G, Vertex> predecessors;

        explicit Visitor(VertexReadWritePropertyMap<G, Vertex> predecessors_) : predecessors(predecessors_) {}

        void tree_edge(Edge edge, const G& graph_) { put(predecessors, target(edge, graph_), source(edge, graph_)); }
    };

    auto predecessor_map = VertexReadWritePropertyMap<G, Vertex>(predecessors);
    auto visitor = Visitor(predecessor_map);

    depth_first_visit(graph, first, last, visitor);

    return predecessors;
}

template<IsGraph G>
auto depth_first_search(const G& graph, const std::vector<VertexIndex>& sources)
{
    return depth_first_search(graph, sources.begin(), sources.end());
}

template<IsGraph G>
auto topological_sort(const G& graph)
{
    using Vertex = typename boost::graph_traits<G>::vertex_descriptor;

    auto result = std::vector<Vertex>();
    auto colors = make_vertex_map_storage(graph, boost::white_color);
    auto color_map = VertexReadWritePropertyMap<G, boost::default_color_type>(colors);

    boost::topological_sort(graph, std::back_inserter(result), boost::color_map(color_map));

    return result;
}

template<IsGraph G, typename Cost, typename SourceInputIterator>
auto dijkstra_shortest_paths_impl(const G& graph, const EdgeMapStorage<G, Cost>& weights, SourceInputIterator first, SourceInputIterator last)
{
    using Vertex = typename boost::graph_traits<G>::vertex_descriptor;

    auto predecessors = make_vertex_map_storage(graph, boost::graph_traits<G>::null_vertex());
    auto distances = make_vertex_map_storage(graph, std::numeric_limits<Cost>::max());
    auto predecessor_map = VertexReadWritePropertyMap<G, Vertex>(predecessors);
    auto distance_map = VertexReadWritePropertyMap<G, Cost>(distances);
    auto weight_map = EdgeReadPropertyMap<G, Cost>(weights);
    auto vertex_index_storage = make_vertex_index_map(graph);
    auto compare = std::less<Cost>();
    auto combine = std::plus<Cost>();
    auto infinity = std::numeric_limits<Cost>::max();
    auto zero = Cost {};

    if constexpr (IsDenseGraphV<G>)
    {
        boost::dijkstra_shortest_paths(graph,
                                       first,
                                       last,
                                       predecessor_map,
                                       distance_map,
                                       weight_map,
                                       vertex_index_storage,
                                       compare,
                                       combine,
                                       infinity,
                                       zero,
                                       boost::default_dijkstra_visitor());
    }
    else
    {
        auto vertex_index_map = VertexIndexPropertyMap<G>(vertex_index_storage);
        boost::dijkstra_shortest_paths(graph,
                                       first,
                                       last,
                                       predecessor_map,
                                       distance_map,
                                       weight_map,
                                       vertex_index_map,
                                       compare,
                                       combine,
                                       infinity,
                                       zero,
                                       boost::default_dijkstra_visitor());
    }

    return std::make_tuple(predecessors, distances);
}

template<IsGraph G, typename Cost, typename SourceInputIterator>
    requires IsDenseGraphV<G>
auto dijkstra_shortest_paths(const G& graph, const std::vector<Cost>& weights, SourceInputIterator first, SourceInputIterator last)
{
    return dijkstra_shortest_paths_impl<G, Cost>(graph, weights, first, last);
}

template<IsGraph G, typename Cost>
    requires IsDenseGraphV<G>
auto dijkstra_shortest_paths(const G& graph, const std::vector<Cost>& weights, const std::vector<VertexIndex>& sources)
{
    return dijkstra_shortest_paths(graph, weights, sources.begin(), sources.end());
}

template<IsGraph G, typename Cost, typename SourceInputIterator>
    requires(!IsDenseGraphV<G>)
auto dijkstra_shortest_paths(const G& graph, const tyr::UnorderedMap<EdgeIndex, Cost>& weights, SourceInputIterator first, SourceInputIterator last)
{
    return dijkstra_shortest_paths_impl<G, Cost>(graph, weights, first, last);
}

template<IsGraph G, typename Cost>
    requires(!IsDenseGraphV<G>)
auto dijkstra_shortest_paths(const G& graph, const tyr::UnorderedMap<EdgeIndex, Cost>& weights, const std::vector<VertexIndex>& sources)
{
    return dijkstra_shortest_paths(graph, weights, sources.begin(), sources.end());
}

template<IsGraph G, typename Cost>
auto floyd_warshall_all_pairs_shortest_paths_impl(const G& graph, const EdgeMapStorage<G, Cost>& weights)
{
    auto distances = DistanceMatrixStorage<G, Cost>();

    if constexpr (IsDenseGraphV<G>)
        distances.assign(graph.get_num_vertices(), std::vector<Cost>(graph.get_num_vertices()));

    auto distance_matrix = BasicMatrix<G, Cost>(distances);
    auto weight_map = EdgeReadPropertyMap<G, Cost>(weights);

    boost::floyd_warshall_all_pairs_shortest_paths(graph, distance_matrix, boost::weight_map(weight_map));

    return distances;
}

template<IsGraph G, typename Cost>
    requires IsDenseGraphV<G>
auto floyd_warshall_all_pairs_shortest_paths(const G& graph, const std::vector<Cost>& weights)
{
    return floyd_warshall_all_pairs_shortest_paths_impl<G, Cost>(graph, weights);
}

template<IsGraph G, typename Cost>
    requires(!IsDenseGraphV<G>)
auto floyd_warshall_all_pairs_shortest_paths(const G& graph, const tyr::UnorderedMap<EdgeIndex, Cost>& weights)
{
    return floyd_warshall_all_pairs_shortest_paths_impl<G, Cost>(graph, weights);
}

}  // namespace runir::graphs::bgl

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif
