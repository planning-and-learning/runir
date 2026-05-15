#ifndef RUNIR_GRAPHS_ALGORITHMS_WEISFEILER_LEMAN_HPP_
#define RUNIR_GRAPHS_ALGORITHMS_WEISFEILER_LEMAN_HPP_

#include "runir/graphs/algorithms/color_refinement.hpp"

#include <array>
#include <cmath>
#include <cstdint>

namespace runir::graphs::weisfeiler_leman
{

using Color = color_refinement::Color;
using ColorList = color_refinement::ColorList;
using BoolList = std::vector<std::uint8_t>;
using TupleColorAssignmentList = std::vector<std::pair<Color, std::size_t>>;

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

    auto identifying_members() const noexcept { return std::tie(color, neighbor_colors); }

    friend auto operator==(const Signature& lhs, const Signature& rhs) noexcept { return lhs.identifying_members() == rhs.identifying_members(); }
};

template<std::size_t K>
class Certificate
{
public:
    using SignatureList = std::vector<std::pair<Signature<K>, Color>>;
    using RoundSummary = std::vector<std::pair<Signature<K>, std::size_t>>;
    using RoundSummaryList = std::vector<RoundSummary>;

private:
    RoundSummaryList m_round_summaries;
    TupleColorAssignmentList m_colors;

public:
    Certificate() = default;

    Certificate(RoundSummaryList round_summaries, TupleColorAssignmentList colors) : m_round_summaries(std::move(round_summaries)), m_colors(std::move(colors))
    {
    }

    auto get_round_summaries() const noexcept -> const RoundSummaryList& { return m_round_summaries; }
    auto get_colors() const noexcept -> const TupleColorAssignmentList& { return m_colors; }

    auto identifying_members() const noexcept { return std::tie(m_colors, m_round_summaries); }
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
    using InitialSignature = std::tuple<ColorList, BoolList, BoolList>;

    const auto n = vertices.size();
    auto vertex_to_dense = tyr::UnorderedMap<VertexIndex, VertexIndex>();
    for (VertexIndex i = 0; i < n; ++i)
        vertex_to_dense.emplace(vertices[i], i);

    auto adjacency = std::vector<BoolList>(n, BoolList(n, false));
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

        auto equalities = BoolList();
        auto adjacencies = BoolList();
        equalities.reserve(K * K);
        adjacencies.reserve(K * K);
        for (std::size_t i = 0; i < K; ++i)
        {
            for (std::size_t j = 0; j < K; ++j)
            {
                equalities.push_back(static_cast<std::uint8_t>(tuple[i] == tuple[j]));
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

    auto round_summaries = typename Certificate<K>::RoundSummaryList();
    auto tuple_signatures = std::vector<std::pair<std::size_t, Signature<K>>>();
    tuple_signatures.reserve(colors.size());
    auto neighbor_colors = ColorTupleList<K>();
    neighbor_colors.reserve(K * n);
    auto interner = color_refinement::SignatureInterner<Signature<K>>();
    auto next_colors = ColorList(colors.size());
    auto changed = true;
    while (changed)
    {
        changed = false;

        tuple_signatures.clear();
        for (std::size_t hash = 0; hash < colors.size(); ++hash)
        {
            const auto tuple = hash_to_tuple<K>(hash, n);

            neighbor_colors.clear();
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

            tuple_signatures.emplace_back(hash, Signature<K> { colors.at(hash), neighbor_colors });
        }

        // Canonicalize tuple order based on signature.
        std::sort(tuple_signatures.begin(),
                  tuple_signatures.end(),
                  [](const auto& lhs, const auto& rhs)
                  {
                      const auto lhs_members = lhs.second.identifying_members();
                      const auto rhs_members = rhs.second.identifying_members();
                      return std::tie(lhs_members, lhs.first) < std::tie(rhs_members, rhs.first);
                  });

        auto round_summary = typename Certificate<K>::RoundSummary();
        for (const auto& [_, signature] : tuple_signatures)
        {
            if (round_summary.empty() || round_summary.back().first.identifying_members() != signature.identifying_members())
                round_summary.emplace_back(signature, 1);
            else
                ++round_summary.back().second;
        }
        round_summaries.push_back(std::move(round_summary));

        interner.clear();
        // Refine tuple colors.
        for (auto& [hash, signature] : tuple_signatures)
        {
            const auto next_color = interner.get_or_create(std::move(signature));
            next_colors[hash] = next_color;
            changed = changed || next_color != colors.at(hash);
        }

        std::swap(colors, next_colors);
    }

    auto dense_colors = TupleColorAssignmentList();
    dense_colors.reserve(colors.size());
    for (std::size_t hash = 0; hash < colors.size(); ++hash)
        dense_colors.emplace_back(colors[hash], hash);
    std::sort(dense_colors.begin(), dense_colors.end());

    return Certificate<K>(std::move(round_summaries), std::move(dense_colors));
}

}  // namespace runir::graphs::weisfeiler_leman

#endif
