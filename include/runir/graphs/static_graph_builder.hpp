#ifndef RUNIR_GRAPHS_STATIC_GRAPH_BUILDER_HPP_
#define RUNIR_GRAPHS_STATIC_GRAPH_BUILDER_HPP_

#include "runir/graphs/edge.hpp"
#include "runir/graphs/property_map.hpp"
#include "runir/graphs/vertex.hpp"

#include <cassert>
#include <limits>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace runir::graphs
{

template<Property VP = std::tuple<>, Property EP = std::tuple<>>
class StaticGraphBuilder
{
public:
    using VertexPropertyType = VP;
    using EdgePropertyType = EP;
    using VertexType = Vertex<StaticGraphBuilder, VP>;
    using VertexList = std::vector<VertexType>;
    using EdgeType = Edge<StaticGraphBuilder, EP>;
    using EdgeList = std::vector<EdgeType>;
    using VertexPropertyMapType = VertexPropertyMap<VP>;
    using EdgePropertyMapType = EdgePropertyMap<EP>;

private:
    VertexList m_vertices;
    EdgeList m_edges;
    VertexPropertyMapType m_vertex_properties;
    EdgePropertyMapType m_edge_properties;

    static auto next_index(std::size_t size) noexcept -> uint_t
    {
        assert(size <= static_cast<std::size_t>(std::numeric_limits<uint_t>::max()));
        return static_cast<uint_t>(size);
    }

    void assert_valid_vertex([[maybe_unused]] VertexIndex vertex) const noexcept { assert(vertex < m_vertices.size()); }
    void assert_valid_edge([[maybe_unused]] EdgeIndex edge) const noexcept { assert(edge < m_edges.size()); }

public:
    StaticGraphBuilder() = default;

    void clear() noexcept
    {
        m_vertices.clear();
        m_edges.clear();
        m_vertex_properties.clear();
        m_edge_properties.clear();
    }

    template<typename P>
        requires std::same_as<std::remove_cvref_t<P>, VP>
    auto add_vertex(P&& property) -> VertexIndex
    {
        const auto index = next_index(m_vertices.size());
        auto [property_index, _] = m_vertex_properties.get_or_create(tyr::Data<VertexProperty<VP>>(std::forward<P>(property)));
        m_vertices.emplace_back(index, property_index, *this);
        return index;
    }

    template<typename P = VP>
        requires std::same_as<P, std::tuple<>>
    auto add_vertex() -> VertexIndex
    {
        return add_vertex(std::tuple<> {});
    }

    auto add_vertex(const VertexType& vertex) -> VertexIndex { return add_vertex(vertex.get_property()); }

    template<typename P>
        requires std::same_as<std::remove_cvref_t<P>, EP>
    auto add_directed_edge(VertexIndex source, VertexIndex target, P&& property) -> EdgeIndex
    {
        assert_valid_vertex(source);
        assert_valid_vertex(target);

        const auto index = next_index(m_edges.size());
        auto [property_index, _] = m_edge_properties.get_or_create(tyr::Data<EdgeProperty<EP>>(std::forward<P>(property)));
        m_edges.emplace_back(index, source, target, property_index, *this);
        return index;
    }

    template<typename P = EP>
        requires std::same_as<P, std::tuple<>>
    auto add_directed_edge(VertexIndex source, VertexIndex target) -> EdgeIndex
    {
        return add_directed_edge(source, target, std::tuple<> {});
    }

    auto add_directed_edge(VertexIndex source, VertexIndex target, const EdgeType& edge) -> EdgeIndex
    {
        return add_directed_edge(source, target, edge.get_property());
    }

    auto add_undirected_edge(VertexIndex lhs, VertexIndex rhs) -> std::pair<EdgeIndex, EdgeIndex>
        requires std::same_as<EP, std::tuple<>>
    {
        return { add_directed_edge(lhs, rhs), add_directed_edge(rhs, lhs) };
    }

    template<typename P>
        requires std::same_as<std::remove_cvref_t<P>, EP>
    auto add_undirected_edge(VertexIndex lhs, VertexIndex rhs, P&& property) -> std::pair<EdgeIndex, EdgeIndex>
    {
        auto reverse_property = property;
        return { add_directed_edge(lhs, rhs, std::forward<P>(property)), add_directed_edge(rhs, lhs, std::move(reverse_property)) };
    }

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
        return get_edge_indices() | std::views::filter([this, vertex](EdgeIndex edge) { return get_source(edge) == vertex; });
    }

    auto get_in_edge_indices(VertexIndex vertex) const noexcept
    {
        assert_valid_vertex(vertex);
        return get_edge_indices() | std::views::filter([this, vertex](EdgeIndex edge) { return get_target(edge) == vertex; });
    }

    auto get_out_degree(VertexIndex vertex) const noexcept -> Degree
    {
        Degree result = 0;
        for (auto edge : get_out_edge_indices(vertex))
        {
            static_cast<void>(edge);
            ++result;
        }
        return result;
    }

    auto get_in_degree(VertexIndex vertex) const noexcept -> Degree
    {
        Degree result = 0;
        for (auto edge : get_in_edge_indices(vertex))
        {
            static_cast<void>(edge);
            ++result;
        }
        return result;
    }

    auto get_out_edges(VertexIndex vertex) const noexcept
    {
        return get_out_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) -> const EdgeType& { return get_edge(edge); });
    }

    auto get_in_edges(VertexIndex vertex) const noexcept
    {
        return get_in_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) -> const EdgeType& { return get_edge(edge); });
    }

    auto get_successor_indices(VertexIndex vertex) const noexcept
    {
        return get_out_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) { return get_target(edge); });
    }

    auto get_predecessor_indices(VertexIndex vertex) const noexcept
    {
        return get_in_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) { return get_source(edge); });
    }

    auto get_successors(VertexIndex vertex) const noexcept
    {
        return get_successor_indices(vertex) | std::views::transform([this](VertexIndex successor) -> const VertexType& { return get_vertex(successor); });
    }

    auto get_predecessors(VertexIndex vertex) const noexcept
    {
        return get_predecessor_indices(vertex)
               | std::views::transform([this](VertexIndex predecessor) -> const VertexType& { return get_vertex(predecessor); });
    }
};

}  // namespace runir::graphs

#endif
