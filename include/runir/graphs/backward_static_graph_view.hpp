#ifndef RUNIR_GRAPHS_BACKWARD_STATIC_GRAPH_VIEW_HPP_
#define RUNIR_GRAPHS_BACKWARD_STATIC_GRAPH_VIEW_HPP_

#include "runir/graphs/static_graph.hpp"

#include <cassert>
#include <ranges>
#include <vector>

namespace runir::graphs
{

template<IsDenseGraph G>
class BackwardStaticGraphView
{
private:
    const G* m_graph;
    EdgeIndexList m_out_edges;
    std::vector<ygg::uint_t> m_out_edge_offsets;

    [[nodiscard]] auto get_graph() const noexcept -> const G&
    {
        assert(m_graph);
        return *m_graph;
    }

    void assert_valid_vertex([[maybe_unused]] VertexIndex vertex) const noexcept { assert(vertex < get_num_vertices()); }

    void initialize()
    {
        m_out_edges.clear();
        m_out_edges.resize(get_graph().get_num_edges());
        m_out_edge_offsets.assign(get_graph().get_num_vertices() + 1, 0);

        for (auto edge : get_graph().get_edge_indices())
            ++m_out_edge_offsets[get_graph().get_target(edge) + 1];

        for (std::size_t i = 1; i < m_out_edge_offsets.size(); ++i)
            m_out_edge_offsets[i] += m_out_edge_offsets[i - 1];

        auto next_offsets = m_out_edge_offsets;
        for (auto edge : get_graph().get_edge_indices())
            m_out_edges[next_offsets[get_graph().get_target(edge)]++] = edge;
    }

public:
    using VertexPropertyType = GraphVertexPropertyType<G>;
    using EdgePropertyType = GraphEdgePropertyType<G>;
    using VertexType = typename G::VertexType;
    using EdgeType = typename G::EdgeType;

    explicit BackwardStaticGraphView(const G& graph) : m_graph(&graph), m_out_edges(), m_out_edge_offsets() { initialize(); }

    [[nodiscard]] auto get_vertices() const noexcept -> const auto& { return get_graph().get_vertices(); }
    [[nodiscard]] auto get_edges() const noexcept -> const auto& { return get_graph().get_edges(); }
    [[nodiscard]] auto get_vertex_property(VertexPropertyIndex<VertexPropertyType> property) const noexcept -> const VertexPropertyType&
    { return get_graph().get_vertex_property(property); }
    [[nodiscard]] auto get_edge_property(EdgePropertyIndex<EdgePropertyType> property) const noexcept -> const EdgePropertyType&
    { return get_graph().get_edge_property(property); }

    [[nodiscard]] auto get_vertex(VertexIndex vertex) const noexcept -> const VertexType& { return get_graph().get_vertex(vertex); }
    [[nodiscard]] auto get_edge(EdgeIndex edge) const noexcept -> const EdgeType& { return get_graph().get_edge(edge); }

    [[nodiscard]] auto get_num_vertices() const noexcept -> std::size_t { return get_graph().get_num_vertices(); }
    [[nodiscard]] auto get_num_edges() const noexcept -> std::size_t { return get_graph().get_num_edges(); }

    [[nodiscard]] auto get_vertex_indices() const noexcept { return get_graph().get_vertex_indices(); }
    [[nodiscard]] auto get_edge_indices() const noexcept { return get_graph().get_edge_indices(); }

    [[nodiscard]] auto get_source(EdgeIndex edge) const noexcept -> VertexIndex { return get_graph().get_target(edge); }
    [[nodiscard]] auto get_target(EdgeIndex edge) const noexcept -> VertexIndex { return get_graph().get_source(edge); }

    [[nodiscard]] auto get_out_edge_indices(VertexIndex vertex) const noexcept
    {
        assert_valid_vertex(vertex);
        const auto begin = m_out_edge_offsets[vertex];
        const auto end = m_out_edge_offsets[vertex + 1];
        return std::ranges::subrange(m_out_edges.begin() + begin, m_out_edges.begin() + end);
    }

    [[nodiscard]] auto get_out_degree(VertexIndex vertex) const noexcept -> Degree
    {
        assert_valid_vertex(vertex);
        return m_out_edge_offsets[vertex + 1] - m_out_edge_offsets[vertex];
    }

    [[nodiscard]] auto get_out_edges(VertexIndex vertex) const noexcept
    {
        return get_out_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) -> const EdgeType& { return get_edge(edge); });
    }

    [[nodiscard]] auto get_successor_indices(VertexIndex vertex) const noexcept
    {
        return get_out_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) { return get_target(edge); });
    }

    [[nodiscard]] auto get_successors(VertexIndex vertex) const noexcept
    {
        return get_successor_indices(vertex) | std::views::transform([this](VertexIndex successor) -> const VertexType& { return get_vertex(successor); });
    }
};

template<IsDenseGraph G>
auto backward(const G& graph)
{ return BackwardStaticGraphView<G>(graph); }

}  // namespace runir::graphs

#endif
