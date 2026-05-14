#ifndef RUNIR_GRAPHS_DYNAMIC_GRAPH_HPP_
#define RUNIR_GRAPHS_DYNAMIC_GRAPH_HPP_

#include "runir/graphs/edge.hpp"
#include "runir/graphs/property_map.hpp"
#include "runir/graphs/vertex.hpp"

#include <cassert>
#include <limits>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

namespace runir::graphs
{

template<Property VP = std::tuple<>, Property EP = std::tuple<>>
class DynamicGraph
{
public:
    using VertexPropertyType = VP;
    using EdgePropertyType = EP;
    using VertexType = Vertex<DynamicGraph, VP>;
    using VertexMap = tyr::UnorderedMap<VertexIndex, VertexType>;
    using EdgeType = Edge<DynamicGraph, EP>;
    using EdgeMap = tyr::UnorderedMap<EdgeIndex, EdgeType>;
    using VertexPropertyMapType = VertexPropertyMap<VP>;
    using EdgePropertyMapType = EdgePropertyMap<EP>;
    using AdjacentEdgeMap = tyr::UnorderedMap<VertexIndex, EdgeIndexSet>;

private:
    VertexMap m_vertices;
    VertexIndexList m_free_vertices;
    uint_t m_next_vertex_index = 0;
    EdgeMap m_edges;
    EdgeIndexList m_free_edges;
    uint_t m_next_edge_index = 0;
    VertexPropertyMapType m_vertex_properties;
    EdgePropertyMapType m_edge_properties;
    AdjacentEdgeMap m_out_edges;
    AdjacentEdgeMap m_in_edges;

    static auto next_index(uint_t& next, auto& free_indices) noexcept -> uint_t
    {
        if (!free_indices.empty())
        {
            const auto index = free_indices.back();
            free_indices.pop_back();
            return index;
        }

        assert(next < std::numeric_limits<uint_t>::max());
        return next++;
    }

    void assert_valid_vertex([[maybe_unused]] VertexIndex vertex) const noexcept { assert(m_vertices.contains(vertex)); }
    void assert_valid_edge([[maybe_unused]] EdgeIndex edge) const noexcept { assert(m_edges.contains(edge)); }

public:
    DynamicGraph() = default;

    void clear() noexcept
    {
        m_vertices.clear();
        m_free_vertices.clear();
        m_next_vertex_index = 0;
        m_edges.clear();
        m_free_edges.clear();
        m_next_edge_index = 0;
        m_vertex_properties.clear();
        m_edge_properties.clear();
        m_out_edges.clear();
        m_in_edges.clear();
    }

    template<typename P>
        requires std::same_as<std::remove_cvref_t<P>, VP>
    auto add_vertex(P&& property) -> VertexIndex
    {
        const auto index = next_index(m_next_vertex_index, m_free_vertices);
        auto [property_index, _] = m_vertex_properties.get_or_create(tyr::Data<VertexProperty<VP>>(std::forward<P>(property)));

        m_vertices.emplace(index, VertexType(index, property_index, *this));
        m_out_edges[index].clear();
        m_in_edges[index].clear();

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

        const auto index = next_index(m_next_edge_index, m_free_edges);
        auto [property_index, _] = m_edge_properties.get_or_create(tyr::Data<EdgeProperty<EP>>(std::forward<P>(property)));

        m_edges.emplace(index, EdgeType(index, source, target, property_index, *this));
        m_out_edges.at(source).insert(index);
        m_in_edges.at(target).insert(index);

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

    void remove_edge(EdgeIndex edge)
    {
        assert_valid_edge(edge);

        const auto source = get_source(edge);
        const auto target = get_target(edge);

        m_out_edges.at(source).erase(edge);
        m_in_edges.at(target).erase(edge);
        m_edges.erase(edge);
        m_free_edges.push_back(edge);
    }

    void remove_vertex(VertexIndex vertex)
    {
        assert_valid_vertex(vertex);

        auto adjacent_edges = EdgeIndexSet {};
        adjacent_edges.insert(m_out_edges.at(vertex).begin(), m_out_edges.at(vertex).end());
        adjacent_edges.insert(m_in_edges.at(vertex).begin(), m_in_edges.at(vertex).end());

        for (auto edge : adjacent_edges)
            remove_edge(edge);

        m_out_edges.erase(vertex);
        m_in_edges.erase(vertex);
        m_vertices.erase(vertex);
        m_free_vertices.push_back(vertex);
    }

    auto contains_vertex(VertexIndex vertex) const noexcept -> bool { return m_vertices.contains(vertex); }
    auto contains_edge(EdgeIndex edge) const noexcept -> bool { return m_edges.contains(edge); }

    auto get_vertices() const noexcept -> const VertexMap& { return m_vertices; }
    auto get_edges() const noexcept -> const EdgeMap& { return m_edges; }
    auto get_vertex_property_map() const noexcept -> const VertexPropertyMapType& { return m_vertex_properties; }
    auto get_edge_property_map() const noexcept -> const EdgePropertyMapType& { return m_edge_properties; }
    auto get_vertex_property(VertexPropertyIndex<VP> property) const noexcept -> const VP& { return m_vertex_properties.get_value(property); }
    auto get_edge_property(EdgePropertyIndex<EP> property) const noexcept -> const EP& { return m_edge_properties.get_value(property); }

    auto get_vertex(VertexIndex vertex) const -> const VertexType&
    {
        assert_valid_vertex(vertex);
        return m_vertices.at(vertex);
    }

    auto get_edge(EdgeIndex edge) const -> const EdgeType&
    {
        assert_valid_edge(edge);
        return m_edges.at(edge);
    }

    auto get_num_vertices() const noexcept -> std::size_t { return m_vertices.size(); }
    auto get_num_edges() const noexcept -> std::size_t { return m_edges.size(); }

    auto get_vertex_indices() const noexcept { return m_vertices | std::views::keys; }

    auto get_edge_indices() const noexcept { return m_edges | std::views::keys; }

    auto get_source(EdgeIndex edge) const -> VertexIndex { return get_edge(edge).get_source(); }
    auto get_target(EdgeIndex edge) const -> VertexIndex { return get_edge(edge).get_target(); }

    auto get_out_edge_indices(VertexIndex vertex) const -> const EdgeIndexSet&
    {
        assert_valid_vertex(vertex);
        return m_out_edges.at(vertex);
    }

    auto get_in_edge_indices(VertexIndex vertex) const -> const EdgeIndexSet&
    {
        assert_valid_vertex(vertex);
        return m_in_edges.at(vertex);
    }

    auto get_out_degree(VertexIndex vertex) const -> Degree
    {
        assert_valid_vertex(vertex);
        return static_cast<Degree>(m_out_edges.at(vertex).size());
    }

    auto get_in_degree(VertexIndex vertex) const -> Degree
    {
        assert_valid_vertex(vertex);
        return static_cast<Degree>(m_in_edges.at(vertex).size());
    }

    auto get_out_edges(VertexIndex vertex) const
    {
        return get_out_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) -> const EdgeType& { return get_edge(edge); });
    }

    auto get_in_edges(VertexIndex vertex) const
    {
        return get_in_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) -> const EdgeType& { return get_edge(edge); });
    }

    auto get_successor_indices(VertexIndex vertex) const
    {
        return get_out_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) { return get_target(edge); });
    }

    auto get_predecessor_indices(VertexIndex vertex) const
    {
        return get_in_edge_indices(vertex) | std::views::transform([this](EdgeIndex edge) { return get_source(edge); });
    }

    auto get_successors(VertexIndex vertex) const
    {
        return get_successor_indices(vertex) | std::views::transform([this](VertexIndex successor) -> const VertexType& { return get_vertex(successor); });
    }

    auto get_predecessors(VertexIndex vertex) const
    {
        return get_predecessor_indices(vertex)
               | std::views::transform([this](VertexIndex predecessor) -> const VertexType& { return get_vertex(predecessor); });
    }
};

}  // namespace runir::graphs

#endif
