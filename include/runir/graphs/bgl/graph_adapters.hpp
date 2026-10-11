#ifndef RUNIR_GRAPHS_BGL_GRAPH_ADAPTERS_HPP_
#define RUNIR_GRAPHS_BGL_GRAPH_ADAPTERS_HPP_

#include "runir/graphs/bgl/graph_traits.hpp"

#include <boost/graph/graph_traits.hpp>
#include <ranges>
#include <utility>

namespace runir::graphs
{

template<IsGraph G>
auto vertices(const G& graph) -> std::pair<typename boost::graph_traits<G>::vertex_iterator, typename boost::graph_traits<G>::vertex_iterator>
{
    auto&& range = graph.get_vertex_indices();
    return { std::ranges::begin(range), std::ranges::end(range) };
}

template<IsGraph G>
auto num_vertices(const G& graph) -> typename boost::graph_traits<G>::vertices_size_type
{
    return graph.get_num_vertices();
}

template<IsGraph G>
auto source(EdgeIndex edge, const G& graph) -> VertexIndex
{
    return graph.get_source(edge);
}

template<IsGraph G>
auto target(EdgeIndex edge, const G& graph) -> VertexIndex
{
    return graph.get_target(edge);
}

template<IsGraph G>
auto out_edges(VertexIndex vertex,
               const G& graph) -> std::pair<typename boost::graph_traits<G>::out_edge_iterator, typename boost::graph_traits<G>::out_edge_iterator>
{
    auto&& range = graph.get_out_edge_indices(vertex);
    return { std::ranges::begin(range), std::ranges::end(range) };
}

template<IsGraph G>
auto out_degree(VertexIndex vertex, const G& graph) -> typename boost::graph_traits<G>::degree_size_type
{
    return graph.get_out_degree(vertex);
}

template<IsGraph G>
auto edges(const G& graph) -> std::pair<typename boost::graph_traits<G>::edge_iterator, typename boost::graph_traits<G>::edge_iterator>
{
    auto&& range = graph.get_edge_indices();
    return { std::ranges::begin(range), std::ranges::end(range) };
}

template<IsGraph G>
auto num_edges(const G& graph) -> typename boost::graph_traits<G>::edges_size_type
{
    return graph.get_num_edges();
}

}  // namespace runir::graphs

#endif
