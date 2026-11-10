#ifndef RUNIR_GRAPHS_BACKWARD_STATIC_GRAPH_VIEW_HPP_
#define RUNIR_GRAPHS_BACKWARD_STATIC_GRAPH_VIEW_HPP_

#include "runir/graphs/static_graph.hpp"

#include <cassert>
#include <ranges>
#include <urd/repr/maps/csr_multimap.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>

namespace runir::graphs
{

template<IsDenseGraph G>
class BackwardStaticGraphView
{
private:
    const G* m_graph;
    /// The graph's edges by target, each target's in edge order: its reversed out edges.
    urd::Store<urd::map<VertexIndex, EdgeIndex>, urd::multi, urd::dense, urd::frozen> m_out_edges;

    [[nodiscard]] auto get_graph() const noexcept -> const G&
    {
        assert(m_graph);
        return *m_graph;
    }

    void assert_valid_vertex([[maybe_unused]] VertexIndex vertex) const noexcept { assert(vertex < get_num_vertices()); }

    void initialize()
    {
        auto reversed = urd::Store<urd::sequence<std::pair<VertexIndex, EdgeIndex>>>();
        reversed.reserve(get_graph().get_num_edges());
        for (auto edge : get_graph().get_edge_indices())
            reversed.emplace_back(get_graph().get_target(edge), edge);
        m_out_edges = decltype(m_out_edges)(reversed.begin(), reversed.end());
    }

public:
    using VertexPropertyType = GraphVertexPropertyType<G>;
    using EdgePropertyType = GraphEdgePropertyType<G>;
    using VertexType = typename G::VertexType;
    using EdgeType = typename G::EdgeType;

    explicit BackwardStaticGraphView(const G& graph) : m_graph(&graph), m_out_edges() { initialize(); }

    [[nodiscard]] auto get_vertices() const noexcept -> const auto& { return get_graph().get_vertices(); }
    [[nodiscard]] auto get_edges() const noexcept -> const auto& { return get_graph().get_edges(); }
    [[nodiscard]] auto get_vertex_property(VertexIndex vertex) const noexcept -> const VertexPropertyType& { return get_graph().get_vertex_property(vertex); }
    [[nodiscard]] auto get_edge_property(EdgeIndex edge) const noexcept -> const EdgePropertyType& { return get_graph().get_edge_property(edge); }

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
        const auto [first, last] = m_out_edges.equal_range(vertex);
        return std::ranges::subrange(first, last) | std::views::values;
    }

    [[nodiscard]] auto get_out_degree(VertexIndex vertex) const noexcept -> Degree
    {
        assert_valid_vertex(vertex);
        return static_cast<Degree>(m_out_edges.count(vertex));
    }

    [[nodiscard]] auto get_out_edges(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_out_edge_indices(vertex), detail::ReadThrough<BackwardStaticGraphView, &BackwardStaticGraphView::get_edge> { this });
    }

    [[nodiscard]] auto get_successor_indices(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_out_edge_indices(vertex), detail::ReadThrough<BackwardStaticGraphView, &BackwardStaticGraphView::get_target> { this });
    }

    [[nodiscard]] auto get_successors(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_successor_indices(vertex), detail::ReadThrough<BackwardStaticGraphView, &BackwardStaticGraphView::get_vertex> { this });
    }
};

}  // namespace runir::graphs

#endif
