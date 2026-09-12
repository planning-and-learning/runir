#ifndef RUNIR_GRAPHS_CYCLE_HPP_
#define RUNIR_GRAPHS_CYCLE_HPP_

#include "runir/graphs/algorithms.hpp"

#include <algorithm>
#include <unordered_map>

namespace runir::graphs
{

template<typename Graph>
class CycleVisitor : public bgl::TraversalVisitor<Graph>
{
private:
    const Graph& m_graph;
    std::unordered_map<VertexIndex, VertexIndex> m_parent;
    VertexIndexList m_cycle;

public:
    explicit CycleVisitor(const Graph& graph) : m_graph(graph) {}

    void tree_edge(EdgeIndex edge) override { m_parent[m_graph.get_target(edge)] = m_graph.get_source(edge); }

    void back_edge(EdgeIndex edge) override
    {
        if (!m_cycle.empty())
            return;

        const auto source = m_graph.get_source(edge);
        const auto target = m_graph.get_target(edge);

        m_cycle.push_back(target);
        for (auto vertex = source; vertex != target; vertex = m_parent.at(vertex))
            m_cycle.push_back(vertex);
        m_cycle.push_back(target);
        std::ranges::reverse(m_cycle);
    }

    const auto& get_cycle() const noexcept { return m_cycle; }
};

template<typename Graph>
auto find_cycle(const Graph& graph) -> VertexIndexList
{
    auto visitor = CycleVisitor<Graph>(graph);
    auto sources = VertexIndexList {};
    sources.reserve(graph.get_num_vertices());
    for (auto vertex : graph.get_vertex_indices())
        sources.push_back(vertex);

    algorithms::depth_first_visit(graph, sources, visitor);
    return visitor.get_cycle();
}

/// An ordered closed path of edge indices; empty iff the graph is acyclic.
/// Parallel edges are resolved to the first matching edge in graph order.
template<typename Graph>
auto find_edge_cycle(const Graph& graph) -> EdgeIndexList
{
    const auto vertices = find_cycle(graph);
    auto result = EdgeIndexList {};
    for (std::size_t position = 1; position < vertices.size(); ++position)
    {
        const auto outgoing = graph.get_out_edge_indices(vertices[position - 1]);
        result.push_back(*std::ranges::find_if(outgoing, [&](auto edge) { return graph.get_target(edge) == vertices[position]; }));
    }
    return result;
}

}  // namespace runir::graphs

#endif
