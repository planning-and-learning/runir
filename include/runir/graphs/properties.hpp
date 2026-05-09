#ifndef RUNIR_GRAPHS_PROPERTIES_HPP_
#define RUNIR_GRAPHS_PROPERTIES_HPP_

#include "runir/graphs/declarations.hpp"

#include <set>

namespace runir::graphs
{

template<IsGraph G>
auto is_loopless(const G& graph) -> bool
{
    for (auto edge : graph.get_edge_indices())
        if (graph.get_source(edge) == graph.get_target(edge))
            return false;
    return true;
}

template<IsGraph G>
auto is_multi_graph(const G& graph) -> bool
{
    auto edges = std::set<std::pair<VertexIndex, VertexIndex>>();
    for (auto edge : graph.get_edge_indices())
    {
        const auto [_, inserted] = edges.emplace(graph.get_source(edge), graph.get_target(edge));
        if (!inserted)
            return true;
    }
    return false;
}

template<IsGraph G>
auto is_symmetric(const G& graph) -> bool
{
    auto edges = std::set<std::pair<VertexIndex, VertexIndex>>();
    for (auto edge : graph.get_edge_indices())
        edges.emplace(graph.get_source(edge), graph.get_target(edge));

    for (const auto& [source, target] : edges)
        if (!edges.contains({ target, source }))
            return false;

    return true;
}

template<IsGraph G>
auto is_simple_undirected(const G& graph) -> bool
{
    return is_loopless(graph) && !is_multi_graph(graph) && is_symmetric(graph);
}

}  // namespace runir::graphs

#endif
