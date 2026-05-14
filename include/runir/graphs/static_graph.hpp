#ifndef RUNIR_GRAPHS_STATIC_GRAPH_HPP_
#define RUNIR_GRAPHS_STATIC_GRAPH_HPP_

#include "runir/graphs/static_graph_builder.hpp"

#include <cassert>
#include <ranges>
#include <vector>

namespace runir::graphs
{

template<Property VP = std::tuple<>, Property EP = std::tuple<>>
class StaticGraph
{
public:
    using VertexPropertyType = VP;
    using EdgePropertyType = EP;
    using VertexType = Vertex<StaticGraph, VP>;
    using VertexList = std::vector<VertexType>;
    using EdgeType = Edge<StaticGraph, EP>;
    using EdgeList = std::vector<EdgeType>;
    using VertexPropertyMapType = VertexPropertyMap<VP>;
    using EdgePropertyMapType = EdgePropertyMap<EP>;
    using EdgeOffsetList = std::vector<uint_t>;

private:
    VertexList m_vertices;
    EdgeList m_edges;
    VertexPropertyMapType m_vertex_properties;
    EdgePropertyMapType m_edge_properties;
    EdgeOffsetList m_out_edge_offsets;

    void assert_valid_vertex([[maybe_unused]] VertexIndex vertex) const noexcept { assert(vertex < m_vertices.size()); }
    void assert_valid_edge([[maybe_unused]] EdgeIndex edge) const noexcept { assert(edge < m_edges.size()); }

    void initialize(const StaticGraphBuilder<VP, EP>& builder)
    {
        m_vertices.clear();
        m_vertices.reserve(builder.get_num_vertices());
        m_edges.clear();
        m_edges.reserve(builder.get_num_edges());
        m_vertex_properties.clear();
        m_edge_properties.clear();
        m_out_edge_offsets.assign(m_vertices.size() + 1, 0);

        for (const auto& vertex : builder.get_vertices())
        {
            auto [property_index, _] = m_vertex_properties.get_or_create(tyr::Data<VertexProperty<VP>>(vertex.get_property()));
            m_vertices.emplace_back(vertex.get_index(), property_index, *this);
        }

        m_out_edge_offsets.assign(m_vertices.size() + 1, 0);

        for (const auto& edge : builder.get_edges())
        {
            assert(edge.get_source() < m_vertices.size());
            ++m_out_edge_offsets[edge.get_source() + 1];
        }

        for (std::size_t i = 1; i < m_out_edge_offsets.size(); ++i)
            m_out_edge_offsets[i] += m_out_edge_offsets[i - 1];

        EdgeIndex index = 0;
        for (auto source : get_vertex_indices())
        {
            for (const auto& edge : builder.get_edges())
            {
                if (edge.get_source() != source)
                    continue;

                auto [property_index, _] = m_edge_properties.get_or_create(tyr::Data<EdgeProperty<EP>>(edge.get_property()));
                m_edges.emplace_back(index++, edge.get_source(), edge.get_target(), property_index, *this);
            }
        }

        assert(m_edges.size() == builder.get_num_edges());
    }

public:
    StaticGraph() : m_vertices(), m_edges(), m_out_edge_offsets(1, 0) {}

    explicit StaticGraph(const StaticGraphBuilder<VP, EP>& builder) { initialize(builder); }

    explicit StaticGraph(StaticGraphBuilder<VP, EP>&& builder) { initialize(builder); }

    auto get_vertices() const noexcept -> const VertexList& { return m_vertices; }
    auto get_edges() const noexcept -> const EdgeList& { return m_edges; }
    auto get_vertex_property_map() const noexcept -> const VertexPropertyMapType& { return m_vertex_properties; }
    auto get_edge_property_map() const noexcept -> const EdgePropertyMapType& { return m_edge_properties; }
    auto get_vertex_property(VertexPropertyIndex<VP> property) const noexcept -> const VP& { return m_vertex_properties.get_value(property); }
    auto get_edge_property(EdgePropertyIndex<EP> property) const noexcept -> const EP& { return m_edge_properties.get_value(property); }

    auto get_vertex(VertexIndex vertex) const noexcept -> const VertexType&
    {
        assert_valid_vertex(vertex);
        return m_vertices[vertex];
    }

    auto get_edge(EdgeIndex edge) const noexcept -> const EdgeType&
    {
        assert_valid_edge(edge);
        return m_edges[edge];
    }

    auto get_num_vertices() const noexcept -> std::size_t { return m_vertices.size(); }
    auto get_num_edges() const noexcept -> std::size_t { return m_edges.size(); }

    auto get_vertex_indices() const noexcept { return std::views::iota(VertexIndex { 0 }, static_cast<VertexIndex>(m_vertices.size())); }
    auto get_edge_indices() const noexcept { return std::views::iota(EdgeIndex { 0 }, static_cast<EdgeIndex>(m_edges.size())); }

    auto get_source(EdgeIndex edge) const noexcept -> VertexIndex { return get_edge(edge).get_source(); }
    auto get_target(EdgeIndex edge) const noexcept -> VertexIndex { return get_edge(edge).get_target(); }

    auto get_out_edge_indices(VertexIndex vertex) const noexcept
    {
        assert_valid_vertex(vertex);
        const auto begin = m_out_edge_offsets[vertex];
        const auto end = m_out_edge_offsets[vertex + 1];
        return std::views::iota(static_cast<EdgeIndex>(begin), static_cast<EdgeIndex>(end));
    }

    auto get_out_degree(VertexIndex vertex) const noexcept -> Degree
    {
        assert_valid_vertex(vertex);
        return m_out_edge_offsets[vertex + 1] - m_out_edge_offsets[vertex];
    }

    auto get_out_edges(VertexIndex vertex) const noexcept
    {
        return get_out_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) -> const EdgeType& { return get_edge(edge); });
    }

    auto get_successor_indices(VertexIndex vertex) const noexcept
    {
        return get_out_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) { return get_target(edge); });
    }

    auto get_successors(VertexIndex vertex) const noexcept
    {
        return get_successor_indices(vertex) | std::views::transform([this](VertexIndex successor) -> const VertexType& { return get_vertex(successor); });
    }
};

}  // namespace runir::graphs

#endif
