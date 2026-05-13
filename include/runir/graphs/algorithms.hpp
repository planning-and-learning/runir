#ifndef RUNIR_GRAPHS_ALGORITHMS_HPP_
#define RUNIR_GRAPHS_ALGORITHMS_HPP_

#include "runir/graphs/algorithms/color_refinement.hpp"
#include "runir/graphs/algorithms/weisfeiler_leman.hpp"
#include "runir/graphs/bgl/algorithms.hpp"

namespace runir::graphs::algorithms
{

template<IsGraph G>
auto strong_components(const G& graph)
{
    return bgl::strong_components(graph);
}

template<IsGraph G>
auto breadth_first_search(const G& graph, const VertexIndexList& sources)
{
    return bgl::breadth_first_search(graph, sources);
}

template<IsGraph G>
auto breadth_first_visit(const G& graph, const VertexIndexList& sources, bgl::TraversalVisitor<G>& visitor)
{
    return bgl::breadth_first_visit(graph, sources, visitor);
}

template<IsGraph G>
auto depth_first_search(const G& graph, const VertexIndexList& sources)
{
    return bgl::depth_first_search(graph, sources);
}

template<IsGraph G>
auto depth_first_visit(const G& graph, const VertexIndexList& sources, bgl::TraversalVisitor<G>& visitor)
{
    return bgl::depth_first_visit(graph, sources, visitor);
}

template<IsGraph G>
auto topological_sort(const G& graph)
{
    return bgl::topological_sort(graph);
}

template<IsGraph G>
auto color_refinement_certificate(const G& graph)
{
    return color_refinement::compute_certificate(graph);
}

template<IsGraph G>
auto weisfeiler_leman_2_certificate(const G& graph)
{
    return weisfeiler_leman::compute_certificate<2>(graph);
}

template<IsGraph G>
auto weisfeiler_leman_3_certificate(const G& graph)
{
    return weisfeiler_leman::compute_certificate<3>(graph);
}

template<IsGraph G>
auto dijkstra_shortest_paths(const G& graph, const bgl::EdgeMapStorage<G, double>& weights, const VertexIndexList& sources)
{
    return bgl::dijkstra_shortest_paths(graph, weights, sources);
}

template<IsGraph G>
auto floyd_warshall_all_pairs_shortest_paths(const G& graph, const bgl::EdgeMapStorage<G, double>& weights)
{
    return bgl::floyd_warshall_all_pairs_shortest_paths(graph, weights);
}

}  // namespace runir::graphs::algorithms

#endif
