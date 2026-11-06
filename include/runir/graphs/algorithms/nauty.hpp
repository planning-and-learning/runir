#ifndef RUNIR_GRAPHS_ALGORITHMS_NAUTY_HPP_
#define RUNIR_GRAPHS_ALGORITHMS_NAUTY_HPP_

#include "runir/graphs/declarations.hpp"
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/graphs/properties.hpp"

#include <algorithm>
#include <memory>
#include <set>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <urd/elements/requirements.hpp>
#include <vector>

namespace runir::graphs::nauty
{

namespace details
{
class SparseGraphImpl;
}

/// A vertex color as an integer: an index (urd::index_of covers integers, enums and index types alike).
template<typename Color>
std::size_t color_index(const Color& color)
{ return urd::index_of<std::remove_cvref_t<Color>>::to_index(color); }

class SparseGraph : public urd::compared_by_identity<SparseGraph>
{
private:
    std::unique_ptr<details::SparseGraphImpl> m_impl;

    SparseGraph(std::size_t nde,
                urd::Store<urd::sequence<std::size_t>> offsets,
                int num_vertices,
                urd::Store<urd::sequence<int>> degrees,
                urd::Store<urd::sequence<int>> edges,
                urd::Store<urd::sequence<int>> labels,
                urd::Store<urd::sequence<int>> partitions,
                urd::Store<urd::sequence<ygg::uint_t>> coloring);

public:
    SparseGraph();

    template<IsGraph G>
    explicit SparseGraph(const G& graph) : SparseGraph()
    {
        auto vertices = VertexIndexList();
        vertices.reserve(graph.get_num_vertices());
        for (auto vertex : graph.get_vertex_indices())
            vertices.push_back(vertex);
        std::sort(vertices.begin(), vertices.end());

        auto vertex_to_dense = urd::Store<urd::map<VertexIndex, ygg::uint_t>>();
        for (ygg::uint_t i = 0; i < vertices.size(); ++i)
            vertex_to_dense.emplace(vertices[i], i);

        if (!is_loopless(graph))
            throw std::runtime_error("runir::graphs::nauty::SparseGraph: loop edges are not supported.");

        if (is_multi_graph(graph))
            throw std::runtime_error("runir::graphs::nauty::SparseGraph: parallel edges are not supported.");

        if (!is_symmetric(graph))
            throw std::runtime_error("runir::graphs::nauty::SparseGraph: directed graphs are not supported.");

        auto edge_set = urd::Store<urd::set<std::pair<ygg::uint_t, ygg::uint_t>>, urd::sorted, urd::stable_refs>();
        for (auto edge : graph.get_edge_indices())
        {
            const auto source = vertex_to_dense.at(graph.get_source(edge));
            const auto target = vertex_to_dense.at(graph.get_target(edge));
            edge_set.emplace(source, target);
        }

        auto offsets = urd::Store<urd::sequence<std::size_t>>();
        auto degrees = urd::Store<urd::sequence<int>>();
        auto edges = urd::Store<urd::sequence<int>>();
        offsets.reserve(vertices.size());
        degrees.reserve(vertices.size());
        edges.reserve(edge_set.size());

        for (ygg::uint_t vertex = 0; vertex < vertices.size(); ++vertex)
        {
            offsets.push_back(edges.size());

            auto degree = 0;
            for (const auto& [source, target] : edge_set)
            {
                if (source != vertex)
                    continue;

                edges.push_back(static_cast<int>(target));
                ++degree;
            }
            degrees.push_back(degree);
        }

        auto color_vertex_pairs = urd::Store<urd::sequence<std::pair<ygg::uint_t, int>>>();
        color_vertex_pairs.reserve(vertices.size());
        for (ygg::uint_t dense_vertex = 0; dense_vertex < vertices.size(); ++dense_vertex)
            color_vertex_pairs.emplace_back(static_cast<ygg::uint_t>(color_index(get_vertex_color(graph.get_vertex(vertices[dense_vertex])))),
                                            static_cast<int>(dense_vertex));
        std::sort(color_vertex_pairs.begin(), color_vertex_pairs.end());

        auto coloring = urd::Store<urd::sequence<ygg::uint_t>>();
        auto labels = urd::Store<urd::sequence<int>>(vertices.size(), 0);
        auto partitions = urd::Store<urd::sequence<int>>(vertices.size(), 0);
        coloring.reserve(vertices.size());
        for (std::size_t i = 0; i < color_vertex_pairs.size(); ++i)
        {
            const auto [color, vertex] = color_vertex_pairs[i];
            coloring.push_back(color);
            labels[i] = vertex;
            partitions[i] = (i + 1 < color_vertex_pairs.size() && color == color_vertex_pairs[i + 1].first) ? 1 : 0;
        }

        *this = SparseGraph(edge_set.size(),
                            std::move(offsets),
                            static_cast<int>(vertices.size()),
                            std::move(degrees),
                            std::move(edges),
                            std::move(labels),
                            std::move(partitions),
                            std::move(coloring));
    }

    SparseGraph(const SparseGraph& other);
    auto operator=(const SparseGraph& other) -> SparseGraph&;
    SparseGraph(SparseGraph&& other) noexcept;
    auto operator=(SparseGraph&& other) noexcept -> SparseGraph&;
    ~SparseGraph();

    auto canonize() -> SparseGraph&;

    [[nodiscard]] auto get_num_directed_edges() const -> std::size_t;
    [[nodiscard]] auto get_num_vertices() const -> int;
    [[nodiscard]] auto get_offsets() const -> const urd::Store<urd::sequence<std::size_t>>&;
    [[nodiscard]] auto get_degrees() const -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_edges() const -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_labels() const -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_partitions() const -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_orbits() const -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_coloring() const -> const urd::Store<urd::sequence<ygg::uint_t>>&;
    [[nodiscard]] auto get_permutation() const -> const urd::Store<urd::sequence<int>>&;
    [[nodiscard]] auto get_inverse_permutation() const -> const urd::Store<urd::sequence<int>>&;

    static constexpr auto identifying_members() noexcept
    {
        return urd::Members<&SparseGraph::get_num_directed_edges,
                            &SparseGraph::get_num_vertices,
                            &SparseGraph::get_offsets,
                            &SparseGraph::get_degrees,
                            &SparseGraph::get_edges,
                            &SparseGraph::get_coloring> {};
    }
};

auto apply_permutation(const urd::Store<urd::sequence<int>>& permutation, urd::Store<urd::sequence<int>>& values) -> urd::Store<urd::sequence<int>>&;
auto compute_label_permutation(const SparseGraph& source, const SparseGraph& target) -> urd::Store<urd::sequence<int>>;
auto compute_permutation(const SparseGraph& source, const SparseGraph& target) -> urd::Store<urd::sequence<int>>;

}  // namespace runir::graphs::nauty

#endif
