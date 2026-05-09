#ifndef RUNIR_GRAPHS_ALGORITHMS_NAUTY_HPP_
#define RUNIR_GRAPHS_ALGORITHMS_NAUTY_HPP_

#include "runir/graphs/declarations.hpp"
#include "runir/graphs/properties.hpp"

#include <algorithm>
#include <memory>
#include <set>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace runir::graphs::nauty
{

namespace details
{
class SparseGraphImpl;
}

class SparseGraph
{
private:
    std::unique_ptr<details::SparseGraphImpl> m_impl;

    SparseGraph(std::size_t nde,
                std::vector<std::size_t> offsets,
                int num_vertices,
                std::vector<int> degrees,
                std::vector<int> edges,
                std::vector<int> labels,
                std::vector<int> partitions,
                std::vector<uint_t> coloring);

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

        auto vertex_to_dense = tyr::UnorderedMap<VertexIndex, uint_t>();
        for (uint_t i = 0; i < vertices.size(); ++i)
            vertex_to_dense.emplace(vertices[i], i);

        if (!is_loopless(graph))
            throw std::runtime_error("runir::graphs::nauty::SparseGraph: loop edges are not supported.");

        if (is_multi_graph(graph))
            throw std::runtime_error("runir::graphs::nauty::SparseGraph: parallel edges are not supported.");

        if (!is_symmetric(graph))
            throw std::runtime_error("runir::graphs::nauty::SparseGraph: directed graphs are not supported.");

        auto edge_set = std::set<std::pair<uint_t, uint_t>>();
        for (auto edge : graph.get_edge_indices())
        {
            const auto source = vertex_to_dense.at(graph.get_source(edge));
            const auto target = vertex_to_dense.at(graph.get_target(edge));
            edge_set.emplace(source, target);
        }

        auto offsets = std::vector<std::size_t>();
        auto degrees = std::vector<int>();
        auto edges = std::vector<int>();
        offsets.reserve(vertices.size());
        degrees.reserve(vertices.size());
        edges.reserve(edge_set.size());

        for (uint_t vertex = 0; vertex < vertices.size(); ++vertex)
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

        auto color_vertex_pairs = std::vector<std::pair<uint_t, int>>();
        color_vertex_pairs.reserve(vertices.size());
        for (uint_t dense_vertex = 0; dense_vertex < vertices.size(); ++dense_vertex)
            color_vertex_pairs.emplace_back(static_cast<uint_t>(graph.get_vertex(vertices[dense_vertex]).get_property_index()), static_cast<int>(dense_vertex));
        std::sort(color_vertex_pairs.begin(), color_vertex_pairs.end());

        auto coloring = std::vector<uint_t>();
        auto labels = std::vector<int>(vertices.size(), 0);
        auto partitions = std::vector<int>(vertices.size(), 0);
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

    auto get_num_directed_edges() const -> std::size_t;
    auto get_num_vertices() const -> int;
    auto get_offsets() const -> const std::vector<std::size_t>&;
    auto get_degrees() const -> const std::vector<int>&;
    auto get_edges() const -> const std::vector<int>&;
    auto get_labels() const -> const std::vector<int>&;
    auto get_partitions() const -> const std::vector<int>&;
    auto get_orbits() const -> const std::vector<int>&;
    auto get_coloring() const -> const std::vector<uint_t>&;
    auto get_permutation() const -> const std::vector<int>&;
    auto get_inverse_permutation() const -> const std::vector<int>&;

    auto identifying_members() const noexcept
    {
        return std::tuple(get_num_directed_edges(),
                          get_num_vertices(),
                          std::cref(get_offsets()),
                          std::cref(get_degrees()),
                          std::cref(get_edges()),
                          std::cref(get_coloring()));
    }
};

auto apply_permutation(const std::vector<int>& permutation, std::vector<int>& values) -> std::vector<int>&;
auto compute_label_permutation(const SparseGraph& source, const SparseGraph& target) -> std::vector<int>;
auto compute_permutation(const SparseGraph& source, const SparseGraph& target) -> std::vector<int>;

auto operator==(const SparseGraph& lhs, const SparseGraph& rhs) noexcept -> bool;

}  // namespace runir::graphs::nauty

#endif
