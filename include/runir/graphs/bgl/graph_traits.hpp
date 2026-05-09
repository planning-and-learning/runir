#ifndef RUNIR_GRAPHS_BGL_GRAPH_TRAITS_HPP_
#define RUNIR_GRAPHS_BGL_GRAPH_TRAITS_HPP_

#include "runir/graphs/declarations.hpp"

#include <boost/graph/graph_traits.hpp>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>

namespace runir::graphs::bgl
{

struct TraversalCategory : boost::vertex_list_graph_tag, boost::incidence_graph_tag, boost::edge_list_graph_tag
{
};

template<typename G>
using VertexIterator = decltype(std::ranges::begin(std::declval<const G&>().get_vertex_indices()));

template<typename G>
using EdgeIterator = decltype(std::ranges::begin(std::declval<const G&>().get_edge_indices()));

template<typename G>
using OutEdgeIterator = decltype(std::ranges::begin(std::declval<const G&>().get_out_edge_indices(std::declval<VertexIndex>())));

}  // namespace runir::graphs::bgl

namespace boost
{

template<runir::graphs::IsGraph G>
struct graph_traits<G>
{
    using vertex_descriptor = runir::graphs::VertexIndex;
    using edge_descriptor = runir::graphs::EdgeIndex;
    using directed_category = directed_tag;
    using edge_parallel_category = allow_parallel_edge_tag;
    using traversal_category = runir::graphs::bgl::TraversalCategory;

    using vertex_iterator = runir::graphs::bgl::VertexIterator<G>;
    using vertices_size_type = std::size_t;

    using out_edge_iterator = runir::graphs::bgl::OutEdgeIterator<G>;
    using degree_size_type = runir::graphs::Degree;

    using edge_iterator = runir::graphs::bgl::EdgeIterator<G>;
    using edges_size_type = std::size_t;

    static constexpr auto null_vertex() noexcept -> vertex_descriptor { return std::numeric_limits<vertex_descriptor>::max(); }
};

template<runir::graphs::IsGraph G>
struct graph_traits<const G> : graph_traits<G>
{
};

}  // namespace boost

#endif
