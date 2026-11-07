#ifndef RUNIR_GRAPHS_STATIC_GRAPH_HPP_
#define RUNIR_GRAPHS_STATIC_GRAPH_HPP_

#include "runir/graphs/static_graph_builder.hpp"

#include <cassert>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <limits>
#include <ranges>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

namespace runir::graphs
{

template<Property VP = std::tuple<>, Property EP = std::tuple<>>
class StaticGraph
{
public:
    using VertexPropertyType = VP;
    using EdgePropertyType = EP;
    using VertexType = Vertex<VP>;
    using EdgeType = Edge<EP>;
    using EdgePropertyMapType = EdgePropertyMap<EP>;

private:
    using VertexList = urd::Store<urd::sequence<VertexType>>;
    using EdgeList = urd::Store<urd::sequence<EdgeType>>;
    using VertexPropertyMapType = VertexPropertyMap<VP>;
    using EdgeOffsetList = urd::Store<urd::sequence<ygg::uint_t>>;

    VertexList m_vertices;
    EdgeList m_edges;
    VertexPropertyMapType m_vertex_properties { urd::layer { 0 } };
    EdgePropertyMapType m_edge_properties { urd::layer { 0 } };
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

        for (const auto& vertex : builder.get_vertices())
        {
            auto [property_index, _] = m_vertex_properties.insert(urd::data<VertexProperty<VP>>(builder.get_vertex_property(vertex.get_index())));
            m_vertices.emplace_back(vertex.get_index(), property_index);
        }

        m_out_edge_offsets.resize(m_vertices.size() + 1);
        EdgeIndex index = 0;
        for (auto source : get_vertex_indices())
        {
            m_out_edge_offsets[source] = index;
            for (const auto& edge : builder.get_out_edges(source))
            {
                auto [property_index, _] = m_edge_properties.insert(urd::data<EdgeProperty<EP>>(builder.get_edge_property(edge.get_index())));
                m_edges.emplace_back(index++, source, edge.get_target(), property_index);
            }
        }
        m_out_edge_offsets.back() = index;

        assert(m_edges.size() == builder.get_num_edges());
    }

public:
    StaticGraph() : m_vertices(), m_edges(), m_out_edge_offsets(1, 0) {}

    explicit StaticGraph(const StaticGraphBuilder<VP, EP>& builder) { initialize(builder); }

    StaticGraph(std::span<const VP> vertices, std::span<const std::tuple<VertexIndex, VertexIndex, EP>> edges) : m_out_edge_offsets(vertices.size() + 1, 0)
    {
        assert(vertices.size() <= static_cast<std::size_t>(std::numeric_limits<VertexIndex>::max()));
        assert(edges.size() <= static_cast<std::size_t>(std::numeric_limits<EdgeIndex>::max()));

        m_vertices.reserve(vertices.size());
        m_edges.reserve(edges.size());
        for (const auto& property : vertices)
        {
            auto [property_index, _] = m_vertex_properties.insert(urd::data<VertexProperty<VP>>(property));
            m_vertices.emplace_back(static_cast<VertexIndex>(m_vertices.size()), property_index);
        }

        for (const auto& [source, target, property] : edges)
        {
            assert_valid_vertex(source);
            assert_valid_vertex(target);
            ++m_out_edge_offsets[source + 1];
        }
        for (std::size_t i = 1; i < m_out_edge_offsets.size(); ++i)
            m_out_edge_offsets[i] += m_out_edge_offsets[i - 1];

        auto next_offsets = m_out_edge_offsets;
        auto ordered_edges = EdgeIndexList(edges.size());
        for (std::size_t i = 0; i < edges.size(); ++i)
            ordered_edges[next_offsets[std::get<0>(edges[i])]++] = static_cast<EdgeIndex>(i);

        for (auto input_index : ordered_edges)
        {
            const auto& [source, target, property] = edges[input_index];
            auto [property_index, _] = m_edge_properties.insert(urd::data<EdgeProperty<EP>>(property));
            m_edges.emplace_back(static_cast<EdgeIndex>(m_edges.size()), source, target, property_index);
        }
    }

    [[nodiscard]] auto get_vertices() const noexcept -> const VertexList& { return m_vertices; }
    [[nodiscard]] auto get_edges() const noexcept -> const EdgeList& { return m_edges; }
    [[nodiscard]] auto get_vertex_property_map() const noexcept -> const VertexPropertyMapType& { return m_vertex_properties; }
    [[nodiscard]] auto get_edge_property_map() const noexcept -> const EdgePropertyMapType& { return m_edge_properties; }
    [[nodiscard]] auto get_vertex_property(VertexIndex vertex) const noexcept -> const VP& { return m_vertex_properties[get_vertex(vertex).get_property_index()].value; }
    [[nodiscard]] auto get_edge_property(EdgeIndex edge) const noexcept -> const EP& { return m_edge_properties[get_edge(edge).get_property_index()].value; }

    [[nodiscard]] auto get_vertex(VertexIndex vertex) const noexcept -> const VertexType&
    {
        assert_valid_vertex(vertex);
        return m_vertices[vertex];
    }

    [[nodiscard]] auto get_edge(EdgeIndex edge) const noexcept -> const EdgeType&
    {
        assert_valid_edge(edge);
        return m_edges[edge];
    }

    [[nodiscard]] auto get_num_vertices() const noexcept -> std::size_t { return m_vertices.size(); }
    [[nodiscard]] auto get_num_edges() const noexcept -> std::size_t { return m_edges.size(); }

    [[nodiscard]] auto get_vertex_indices() const noexcept { return std::views::iota(VertexIndex { 0 }, static_cast<VertexIndex>(m_vertices.size())); }
    [[nodiscard]] auto get_edge_indices() const noexcept { return std::views::iota(EdgeIndex { 0 }, static_cast<EdgeIndex>(m_edges.size())); }

    [[nodiscard]] auto get_source(EdgeIndex edge) const noexcept -> VertexIndex { return get_edge(edge).get_source(); }
    [[nodiscard]] auto get_target(EdgeIndex edge) const noexcept -> VertexIndex { return get_edge(edge).get_target(); }

    [[nodiscard]] auto get_out_edge_indices(VertexIndex vertex) const noexcept
    {
        assert_valid_vertex(vertex);
        const auto begin = m_out_edge_offsets[vertex];
        const auto end = m_out_edge_offsets[vertex + 1];
        return std::views::iota(static_cast<EdgeIndex>(begin), static_cast<EdgeIndex>(end));
    }

    [[nodiscard]] auto get_out_degree(VertexIndex vertex) const noexcept -> Degree
    {
        assert_valid_vertex(vertex);
        return m_out_edge_offsets[vertex + 1] - m_out_edge_offsets[vertex];
    }

    [[nodiscard]] auto get_out_edges(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_out_edge_indices(vertex), detail::ReadThrough<StaticGraph, &StaticGraph::get_edge> { this });
    }

    [[nodiscard]] auto get_successor_indices(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_out_edge_indices(vertex), detail::ReadThrough<StaticGraph, &StaticGraph::get_target> { this });
    }

    [[nodiscard]] auto get_successors(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_successor_indices(vertex), detail::ReadThrough<StaticGraph, &StaticGraph::get_vertex> { this });
    }
};

}  // namespace runir::graphs

#endif
