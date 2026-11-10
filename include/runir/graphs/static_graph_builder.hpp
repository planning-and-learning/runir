#ifndef RUNIR_GRAPHS_STATIC_GRAPH_BUILDER_HPP_
#define RUNIR_GRAPHS_STATIC_GRAPH_BUILDER_HPP_

#include "runir/graphs/declarations.hpp"
#include "runir/graphs/edge.hpp"
#include "runir/graphs/property_map.hpp"
#include "runir/graphs/vertex.hpp"

#include <cassert>
#include <limits>
#include <ranges>
#include <span>
#include <type_traits>
#include <urd/repr/maps/dense_multimap.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
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
    using VertexType = Vertex<VP>;
    using EdgeType = Edge<EP>;
    using EdgePropertyMapType = EdgePropertyMap<EP>;

private:
    using VertexList = urd::Store<urd::sequence<VertexType>>;
    using EdgeList = urd::Store<urd::sequence<EdgeType>>;
    using VertexPropertyMapType = VertexPropertyMap<VP>;
    VertexList m_vertices;
    EdgeList m_edges;
    /// Each vertex's out and in edges, in the order added.
    urd::Store<urd::map<VertexIndex, EdgeIndex>, urd::multi, urd::dense> m_out_edges;
    urd::Store<urd::map<VertexIndex, EdgeIndex>, urd::multi, urd::dense> m_in_edges;
    VertexPropertyMapType m_vertex_properties { urd::layer { 0 } };
    EdgePropertyMapType m_edge_properties { urd::layer { 0 } };

    static auto next_index(std::size_t size) noexcept -> ygg::uint_t
    {
        assert(size <= static_cast<std::size_t>(std::numeric_limits<ygg::uint_t>::max()));
        return static_cast<ygg::uint_t>(size);
    }

    void assert_valid_vertex([[maybe_unused]] VertexIndex vertex) const noexcept { assert(vertex < m_vertices.size()); }
    void assert_valid_edge([[maybe_unused]] EdgeIndex edge) const noexcept { assert(edge < m_edges.size()); }

public:
    void clear() noexcept
    {
        m_vertices.clear();
        m_edges.clear();
        m_out_edges.clear();
        m_in_edges.clear();
        m_vertex_properties.clear();
        m_edge_properties.clear();
    }

    template<typename P>
        requires std::same_as<std::remove_cvref_t<P>, VP>
    auto add_vertex(P&& property) -> VertexIndex
    {
        const auto index = next_index(m_vertices.size());
        auto [property_index, _] = m_vertex_properties.insert(urd::data<VertexProperty<VP>>(std::forward<P>(property)));
        m_vertices.emplace_back(index, property_index);
        return index;
    }

    template<typename P = VP>
        requires std::same_as<P, std::tuple<>>
    auto add_vertex() -> VertexIndex
    { return add_vertex(std::tuple<> {}); }

    template<typename P>
        requires std::same_as<std::remove_cvref_t<P>, EP>
    auto add_directed_edge(VertexIndex source, VertexIndex target, P&& property) -> EdgeIndex
    {
        assert_valid_vertex(source);
        assert_valid_vertex(target);

        const auto index = next_index(m_edges.size());
        auto [property_index, _] = m_edge_properties.insert(urd::data<EdgeProperty<EP>>(std::forward<P>(property)));
        m_edges.emplace_back(index, source, target, property_index);
        m_out_edges.insert({ source, index });
        m_in_edges.insert({ target, index });
        return index;
    }

    template<typename P = EP>
        requires std::same_as<P, std::tuple<>>
    auto add_directed_edge(VertexIndex source, VertexIndex target) -> EdgeIndex
    { return add_directed_edge(source, target, std::tuple<> {}); }

    auto add_undirected_edge(VertexIndex lhs, VertexIndex rhs) -> std::pair<EdgeIndex, EdgeIndex>
        requires std::same_as<EP, std::tuple<>>
    { return { add_directed_edge(lhs, rhs), add_directed_edge(rhs, lhs) }; }

    template<typename P>
        requires std::same_as<std::remove_cvref_t<P>, EP>
    auto add_undirected_edge(VertexIndex lhs, VertexIndex rhs, P&& property) -> std::pair<EdgeIndex, EdgeIndex>
    {
        auto reverse_property = property;
        return { add_directed_edge(lhs, rhs, std::forward<P>(property)), add_directed_edge(rhs, lhs, std::move(reverse_property)) };
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
        const auto [first, last] = m_out_edges.equal_range(vertex);
        return std::ranges::subrange(first, last) | std::views::values;
    }

    [[nodiscard]] auto get_in_edge_indices(VertexIndex vertex) const noexcept
    {
        assert_valid_vertex(vertex);
        const auto [first, last] = m_in_edges.equal_range(vertex);
        return std::ranges::subrange(first, last) | std::views::values;
    }

    [[nodiscard]] auto get_out_degree(VertexIndex vertex) const noexcept -> Degree
    {
        assert_valid_vertex(vertex);
        return static_cast<Degree>(m_out_edges.count(vertex));
    }

    [[nodiscard]] auto get_in_degree(VertexIndex vertex) const noexcept -> Degree
    {
        assert_valid_vertex(vertex);
        return static_cast<Degree>(m_in_edges.count(vertex));
    }

    [[nodiscard]] auto get_out_edges(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_out_edge_indices(vertex), detail::ReadThrough<StaticGraphBuilder, &StaticGraphBuilder::get_edge> { this });
    }

    [[nodiscard]] auto get_in_edges(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_in_edge_indices(vertex), detail::ReadThrough<StaticGraphBuilder, &StaticGraphBuilder::get_edge> { this });
    }

    [[nodiscard]] auto get_successor_indices(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_out_edge_indices(vertex), detail::ReadThrough<StaticGraphBuilder, &StaticGraphBuilder::get_target> { this });
    }

    [[nodiscard]] auto get_predecessor_indices(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_in_edge_indices(vertex), detail::ReadThrough<StaticGraphBuilder, &StaticGraphBuilder::get_source> { this });
    }

    [[nodiscard]] auto get_successors(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_successor_indices(vertex), detail::ReadThrough<StaticGraphBuilder, &StaticGraphBuilder::get_vertex> { this });
    }

    [[nodiscard]] auto get_predecessors(VertexIndex vertex) const noexcept
    {
        return urd::transform(get_predecessor_indices(vertex), detail::ReadThrough<StaticGraphBuilder, &StaticGraphBuilder::get_vertex> { this });
    }
};

}  // namespace runir::graphs

#endif
