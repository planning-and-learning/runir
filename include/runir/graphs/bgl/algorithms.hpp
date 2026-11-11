#ifndef RUNIR_GRAPHS_BGL_ALGORITHMS_HPP_
#define RUNIR_GRAPHS_BGL_ALGORITHMS_HPP_

#include "runir/graphs/bgl/graph_adapters.hpp"

#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

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
#include <boost/property_map/function_property_map.hpp>
#include <boost/property_map/property_map.hpp>
#include <concepts>
#include <functional>
#include <iterator>
#include <limits>
#include <queue>
#include <type_traits>

namespace runir::graphs::bgl
{

template<IsGraph G>
class TraversalVisitor
{
public:
    TraversalVisitor() = default;
    TraversalVisitor(const TraversalVisitor&) = default;
    TraversalVisitor& operator=(const TraversalVisitor&) = default;
    TraversalVisitor(TraversalVisitor&&) noexcept = default;
    TraversalVisitor& operator=(TraversalVisitor&&) noexcept = default;
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

/// What an algorithm returns per vertex (per edge): a sequence by index in a dense graph, else a map by index.
template<typename G, typename Value>
using VertexMapStorage = std::conditional_t<IsDenseGraph<G>, urd::Store<urd::sequence<Value>>, urd::Store<urd::map<VertexIndex, Value>>>;

template<typename G, typename Value>
using EdgeMapStorage = std::conditional_t<IsDenseGraph<G>, urd::Store<urd::sequence<Value>>, urd::Store<urd::map<EdgeIndex, Value>>>;

template<typename G, typename Value>
using DistanceMatrixStorage = std::conditional_t<IsDenseGraph<G>,
                                                 urd::Store<urd::sequence<urd::Store<urd::sequence<Value>>>>,
                                                 urd::Store<urd::map<VertexIndex, urd::Store<urd::map<VertexIndex, Value>>>>>;

/// A Boost property map over a VertexMapStorage or EdgeMapStorage.
template<typename Value>
auto make_property_map(urd::Store<urd::sequence<Value>>& values)
{ return boost::make_iterator_property_map(values.begin(), boost::typed_identity_property_map<ygg::uint_t>()); }

template<typename Value>
auto make_property_map(const urd::Store<urd::sequence<Value>>& values)
{ return boost::make_iterator_property_map(values.begin(), boost::typed_identity_property_map<ygg::uint_t>()); }

template<typename Value>
auto make_property_map(urd::Store<urd::map<ygg::uint_t, Value>>& values)
{ return boost::associative_property_map<urd::Store<urd::map<ygg::uint_t, Value>>>(values); }

template<typename Value>
auto make_property_map(const urd::Store<urd::map<ygg::uint_t, Value>>& values)
{ return boost::const_associative_property_map<urd::Store<urd::map<ygg::uint_t, Value>>>(values); }

/// Calls `f` with Boost's vertex index map: the vertex itself in a dense graph, else its dense index (dense_vertex_indices).
template<IsGraph G, typename F>
decltype(auto) with_vertex_index_map(const G& graph, F&& f)
{
    if constexpr (IsDenseGraph<G>)
        return f(boost::typed_identity_property_map<VertexIndex>());
    else
    {
        const auto vertices = dense_vertex_indices(graph);
        return f(boost::make_function_property_map<VertexIndex>([&vertices](VertexIndex vertex)
                                                                { return static_cast<VertexIndex>(vertices.position(vertices.find(vertex))); }));
    }
}

template<IsGraph G, std::copyable Value>
auto make_vertex_map_storage(const G& graph, const Value& value)
{
    auto result = VertexMapStorage<G, Value>();

    if constexpr (IsDenseGraph<G>)
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
    const auto count = with_vertex_index_map(
        graph,
        [&](auto vertex_index_map) { return boost::strong_components(graph, make_property_map(components), boost::vertex_index_map(vertex_index_map)); });
    return std::make_pair(count, components);
}

/// With a Boost visitor; a TraversalVisitor goes by reference through the overload below.
template<IsGraph G, typename Visitor>
    requires(!std::derived_from<Visitor, TraversalVisitor<G>>)
auto breadth_first_visit(const G& graph, const VertexIndexList& sources, Visitor visitor)
{
    auto colors = make_vertex_map_storage(graph, boost::white_color);
    auto buffer = boost::queue<VertexIndex>();

    boost::breadth_first_search(graph, sources.begin(), sources.end(), buffer, visitor, make_property_map(colors));

    return colors;
}

template<IsGraph G>
auto breadth_first_visit(const G& graph, const VertexIndexList& sources, TraversalVisitor<G>& visitor)
{ return breadth_first_visit(graph, sources, BoostTraversalVisitor<G>(visitor)); }

template<IsGraph G>
auto breadth_first_search(const G& graph, const VertexIndexList& sources)
{
    using Distance = ygg::uint_t;

    auto predecessors = make_vertex_map_storage(graph, boost::graph_traits<G>::null_vertex());
    auto distances = make_vertex_map_storage(graph, std::numeric_limits<Distance>::max());
    for (auto source : sources)
        distances[source] = 0;

    using PredecessorMap = decltype(make_property_map(predecessors));
    using DistanceMap = decltype(make_property_map(distances));

    struct Visitor : boost::bfs_visitor<>
    {
        PredecessorMap predecessors;
        DistanceMap distances;

        Visitor(PredecessorMap predecessors_, DistanceMap distances_) : predecessors(predecessors_), distances(distances_) {}

        void tree_edge(EdgeIndex edge, const G& graph_)
        {
            const auto lhs = source(edge, graph_);
            const auto rhs = target(edge, graph_);
            put(predecessors, rhs, lhs);
            put(distances, rhs, get(distances, lhs) + 1);
        }
    };

    breadth_first_visit(graph, sources, Visitor(make_property_map(predecessors), make_property_map(distances)));

    return std::make_tuple(predecessors, distances);
}

/// With a Boost visitor; a TraversalVisitor goes by reference through the overload below.
template<IsGraph G, typename Visitor>
    requires(!std::derived_from<Visitor, TraversalVisitor<G>>)
auto depth_first_visit(const G& graph, const VertexIndexList& sources, Visitor visitor)
{
    auto colors = make_vertex_map_storage(graph, boost::white_color);
    auto color_map = make_property_map(colors);

    for (auto source : sources)
        if (get(color_map, source) == boost::white_color)
            boost::depth_first_search(graph, visitor, color_map, source);

    return colors;
}

template<IsGraph G>
auto depth_first_visit(const G& graph, const VertexIndexList& sources, TraversalVisitor<G>& visitor)
{ return depth_first_visit(graph, sources, BoostTraversalVisitor<G>(visitor)); }

template<IsGraph G>
auto depth_first_search(const G& graph, const VertexIndexList& sources)
{
    auto predecessors = make_vertex_map_storage(graph, boost::graph_traits<G>::null_vertex());

    using PredecessorMap = decltype(make_property_map(predecessors));

    struct Visitor : boost::dfs_visitor<>
    {
        PredecessorMap predecessors;

        explicit Visitor(PredecessorMap predecessors_) : predecessors(predecessors_) {}

        void tree_edge(EdgeIndex edge, const G& graph_) { put(predecessors, target(edge, graph_), source(edge, graph_)); }
    };

    depth_first_visit(graph, sources, Visitor(make_property_map(predecessors)));

    return predecessors;
}

template<IsGraph G>
auto topological_sort(const G& graph)
{
    auto result = urd::Store<urd::sequence<VertexIndex>>();
    auto colors = make_vertex_map_storage(graph, boost::white_color);

    boost::topological_sort(graph, std::back_inserter(result), boost::color_map(make_property_map(colors)));

    return result;
}

/// Cost is not deduced (weights are EdgeMapStorage<G, Cost>): name it.
template<IsGraph G, typename Cost>
auto dijkstra_shortest_paths(const G& graph, const EdgeMapStorage<G, Cost>& weights, const VertexIndexList& sources)
{
    auto predecessors = make_vertex_map_storage(graph, boost::graph_traits<G>::null_vertex());
    auto distances = make_vertex_map_storage(graph, std::numeric_limits<Cost>::max());

    with_vertex_index_map(graph,
                          [&](auto vertex_index_map)
                          {
                              boost::dijkstra_shortest_paths(graph,
                                                             sources.begin(),
                                                             sources.end(),
                                                             make_property_map(predecessors),
                                                             make_property_map(distances),
                                                             make_property_map(weights),
                                                             vertex_index_map,
                                                             std::less<Cost>(),
                                                             std::plus<Cost>(),
                                                             std::numeric_limits<Cost>::max(),
                                                             Cost {},
                                                             boost::default_dijkstra_visitor());
                          });

    return std::make_tuple(predecessors, distances);
}

/// Cost is not deduced (weights are EdgeMapStorage<G, Cost>): name it.
template<IsGraph G, typename Cost>
auto floyd_warshall_all_pairs_shortest_paths(const G& graph, const EdgeMapStorage<G, Cost>& weights)
{
    auto distances = DistanceMatrixStorage<G, Cost>();

    if constexpr (IsDenseGraph<G>)
        distances.assign(graph.get_num_vertices(), urd::Store<urd::sequence<Cost>>(graph.get_num_vertices()));

    boost::floyd_warshall_all_pairs_shortest_paths(graph, distances, boost::weight_map(make_property_map(weights)));

    return distances;
}

}  // namespace runir::graphs::bgl

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif
