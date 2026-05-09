#ifndef RUNIR_GRAPHS_ALGORITHMS_WEISFEILER_LEMAN_HPP_
#define RUNIR_GRAPHS_ALGORITHMS_WEISFEILER_LEMAN_HPP_

#include "runir/graphs/algorithms/color_refinement.hpp"

#include <array>
#include <cmath>

namespace runir::graphs::weisfeiler_leman
{

using Color = color_refinement::Color;
using ColorList = color_refinement::ColorList;

template<std::size_t K>
using VertexTuple = std::array<VertexIndex, K>;

template<std::size_t K>
using ColorTuple = std::array<Color, K>;

template<std::size_t K>
using ColorTupleList = std::vector<ColorTuple<K>>;

template<std::size_t K>
struct Signature
{
    Color color {};
    ColorTupleList<K> neighbor_colors;

    auto operator==(const Signature&) const -> bool = default;
    auto operator<=>(const Signature&) const = default;
};

template<std::size_t K>
class Certificate
{
public:
    using SignatureList = std::vector<std::pair<Signature<K>, Color>>;

private:
    SignatureList m_refinement_colors;
    ColorList m_colors;

public:
    Certificate() = default;

    Certificate(SignatureList refinement_colors, ColorList colors) : m_refinement_colors(std::move(refinement_colors)), m_colors(std::move(colors)) {}

    auto get_refinement_colors() const noexcept -> const SignatureList& { return m_refinement_colors; }
    auto get_colors() const noexcept -> const ColorList& { return m_colors; }

    auto identifying_members() const noexcept { return std::tie(m_refinement_colors, m_colors); }
};

template<std::size_t K>
auto num_tuples(std::size_t num_vertices) -> std::size_t
{
    std::size_t result = 1;
    for (std::size_t i = 0; i < K; ++i)
        result *= num_vertices;
    return result;
}

template<std::size_t K>
auto tuple_to_hash(const VertexTuple<K>& tuple, std::size_t num_vertices) -> std::size_t
{
    std::size_t hash = 0;
    std::size_t factor = 1;
    for (auto vertex : tuple)
    {
        hash += factor * vertex;
        factor *= num_vertices;
    }
    return hash;
}

template<std::size_t K>
auto hash_to_tuple(std::size_t hash, std::size_t num_vertices) -> VertexTuple<K>
{
    auto result = VertexTuple<K>();
    for (std::size_t i = 0; i < K; ++i)
    {
        result[i] = static_cast<VertexIndex>(hash % num_vertices);
        hash /= num_vertices;
    }
    return result;
}

template<std::size_t K, IsGraph G>
auto initial_tuple_colors(const G& graph, const VertexIndexList& vertices)
{
    using InitialSignature = std::tuple<ColorList, std::vector<bool>, std::vector<bool>>;

    const auto n = vertices.size();
    auto vertex_to_dense = tyr::UnorderedMap<VertexIndex, VertexIndex>();
    for (VertexIndex i = 0; i < n; ++i)
        vertex_to_dense.emplace(vertices[i], i);

    auto adjacency = std::vector<std::vector<bool>>(n, std::vector<bool>(n, false));
    for (auto vertex : vertices)
        for (auto edge : graph.get_out_edge_indices(vertex))
            adjacency.at(vertex_to_dense.at(vertex)).at(vertex_to_dense.at(graph.get_target(edge))) = true;

    auto interner = color_refinement::SignatureInterner<InitialSignature>();
    auto colors = ColorList(num_tuples<K>(n));

    for (std::size_t hash = 0; hash < colors.size(); ++hash)
    {
        const auto tuple = hash_to_tuple<K>(hash, n);

        auto vertex_colors = ColorList();
        vertex_colors.reserve(K);
        for (auto dense_vertex : tuple)
            vertex_colors.push_back(static_cast<Color>(graph.get_vertex(vertices[dense_vertex]).get_property_index()));

        auto equalities = std::vector<bool>();
        auto adjacencies = std::vector<bool>();
        equalities.reserve(K * K);
        adjacencies.reserve(K * K);
        for (std::size_t i = 0; i < K; ++i)
        {
            for (std::size_t j = 0; j < K; ++j)
            {
                equalities.push_back(tuple[i] == tuple[j]);
                adjacencies.push_back(adjacency.at(tuple[i]).at(tuple[j]));
            }
        }

        colors[hash] = interner.get_or_create(InitialSignature(std::move(vertex_colors), std::move(equalities), std::move(adjacencies)));
    }

    return colors;
}

template<std::size_t K, IsGraph G>
auto compute_certificate(const G& graph)
{
    const auto vertices = color_refinement::to_dense_vertex_order(graph);
    const auto n = vertices.size();
    auto colors = initial_tuple_colors<K>(graph, vertices);

    auto refinement_colors = typename Certificate<K>::SignatureList();
    auto changed = true;
    while (changed)
    {
        changed = false;

        auto interner = color_refinement::SignatureInterner<Signature<K>>();
        auto next_colors = ColorList(colors.size());
        for (std::size_t hash = 0; hash < colors.size(); ++hash)
        {
            const auto tuple = hash_to_tuple<K>(hash, n);

            auto neighbor_colors = ColorTupleList<K>();
            neighbor_colors.reserve(K * n);
            for (std::size_t replacement_position = 0; replacement_position < K; ++replacement_position)
            {
                for (VertexIndex replacement = 0; replacement < n; ++replacement)
                {
                    auto neighbor_tuple = tuple;
                    neighbor_tuple[replacement_position] = replacement;

                    auto color_tuple = ColorTuple<K>();
                    for (std::size_t color_position = 0; color_position < K; ++color_position)
                    {
                        auto color_probe = tuple;
                        color_probe[color_position] = replacement;
                        color_tuple[color_position] = colors.at(tuple_to_hash<K>(color_probe, n));
                    }
                    neighbor_colors.push_back(color_tuple);
                }
            }
            std::sort(neighbor_colors.begin(), neighbor_colors.end());

            auto signature = Signature<K> { colors.at(hash), std::move(neighbor_colors) };
            const auto next_color = interner.get_or_create(std::move(signature));
            next_colors[hash] = next_color;
            changed = changed || next_color != colors.at(hash);
        }

        refinement_colors = std::move(interner).release();
        colors = std::move(next_colors);
    }

    return Certificate<K>(std::move(refinement_colors), std::move(colors));
}

}  // namespace runir::graphs::weisfeiler_leman

#endif
