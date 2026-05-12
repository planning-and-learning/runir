#ifndef RUNIR_GRAPHS_ALGORITHMS_COLOR_REFINEMENT_HPP_
#define RUNIR_GRAPHS_ALGORITHMS_COLOR_REFINEMENT_HPP_

#include "runir/graphs/declarations.hpp"

#include <algorithm>
#include <tuple>
#include <vector>

namespace runir::graphs::color_refinement
{

using Color = uint_t;
using ColorList = std::vector<Color>;
using ColorAssignmentList = std::vector<std::pair<Color, VertexIndex>>;

template<IsGraph G>
using Coloring = std::conditional_t<IsDenseGraph<G>, ColorList, tyr::UnorderedMap<VertexIndex, Color>>;

template<typename VertexProperty>
class Certificate
{
public:
    using Signature = std::pair<Color, ColorList>;
    using SignatureList = std::vector<std::pair<Signature, Color>>;
    using RoundSummary = std::vector<std::pair<Signature, std::size_t>>;
    using RoundSummaryList = std::vector<RoundSummary>;

private:
    RoundSummaryList m_round_summaries;
    ColorAssignmentList m_colors;

public:
    Certificate() = default;

    Certificate(RoundSummaryList round_summaries, ColorAssignmentList colors) : m_round_summaries(std::move(round_summaries)), m_colors(std::move(colors))
    {
    }

    auto get_round_summaries() const noexcept -> const RoundSummaryList& { return m_round_summaries; }
    auto get_colors() const noexcept -> const ColorAssignmentList& { return m_colors; }

    auto identifying_members() const noexcept { return std::tie(m_colors, m_round_summaries); }
};

template<typename Signature>
class SignatureInterner
{
private:
    std::vector<std::pair<Signature, Color>> m_data;
    tyr::UnorderedMap<Signature, Color> m_index;

public:
    void clear()
    {
        m_data.clear();
        m_index.clear();
    }

    auto get_or_create(Signature signature) -> Color
    {
        const auto color = static_cast<Color>(m_data.size());
        const auto [it, inserted] = m_index.emplace(std::move(signature), color);
        if (!inserted)
            return it->second;

        m_data.emplace_back(it->first, color);
        return color;
    }

    auto release() && { return std::move(m_data); }
};

template<IsGraph G>
auto to_dense_vertex_order(const G& graph)
{
    auto result = VertexIndexList();
    result.reserve(graph.get_num_vertices());
    for (auto vertex : graph.get_vertex_indices())
        result.push_back(vertex);
    std::sort(result.begin(), result.end());
    return result;
}

template<IsGraph G>
auto make_coloring_storage(const G& graph)
{
    auto result = Coloring<G>();
    if constexpr (IsDenseGraph<G>)
        result.resize(graph.get_num_vertices());
    else
        for (auto vertex : graph.get_vertex_indices())
            result.emplace(vertex, Color {});
    return result;
}

template<IsGraph G>
auto get_color(const Coloring<G>& colors, VertexIndex vertex) -> Color
{
    if constexpr (IsDenseGraph<G>)
        return colors.at(vertex);
    else
        return colors.at(vertex);
}

template<IsGraph G>
void set_color(Coloring<G>& colors, VertexIndex vertex, Color color)
{
    if constexpr (IsDenseGraph<G>)
        colors.at(vertex) = color;
    else
        colors[vertex] = color;
}

template<IsGraph G>
auto compute_certificate(const G& graph)
{
    using VertexProperty = typename G::VertexPropertyType;
    using Result = Certificate<VertexProperty>;
    using Signature = typename Result::Signature;

    const auto vertices = to_dense_vertex_order(graph);
    auto colors = make_coloring_storage(graph);

    for (auto vertex : vertices)
    {
        const auto color = static_cast<Color>(graph.get_vertex(vertex).get_property_index());
        set_color<G>(colors, vertex, color);
    }

    auto round_summaries = typename Result::RoundSummaryList();
    auto vertex_signatures = std::vector<std::pair<VertexIndex, Signature>>();
    vertex_signatures.reserve(vertices.size());
    auto next_colors = make_coloring_storage(graph);
    auto neighbor_colors = ColorList();
    auto interner = SignatureInterner<Signature>();
    auto changed = true;
    while (changed)
    {
        changed = false;

        vertex_signatures.clear();
        for (auto vertex : vertices)
        {
            neighbor_colors.clear();
            neighbor_colors.reserve(graph.get_out_degree(vertex));
            for (auto edge : graph.get_out_edge_indices(vertex))
                neighbor_colors.push_back(get_color<G>(colors, graph.get_target(edge)));
            std::sort(neighbor_colors.begin(), neighbor_colors.end());

            vertex_signatures.emplace_back(vertex, Signature(get_color<G>(colors, vertex), neighbor_colors));
        }

        // Canonicalize vertex order based on signature.
        std::sort(vertex_signatures.begin(), vertex_signatures.end(), [](const auto& lhs, const auto& rhs) {
            return std::tie(lhs.second, lhs.first) < std::tie(rhs.second, rhs.first);
        });

        auto round_summary = typename Result::RoundSummary();
        for (const auto& [_, signature] : vertex_signatures)
        {
            if (round_summary.empty() || round_summary.back().first != signature)
                round_summary.emplace_back(signature, 1);
            else
                ++round_summary.back().second;
        }
        round_summaries.push_back(std::move(round_summary));

        interner.clear();
        // Refine vertex colors.
        for (auto& [vertex, signature] : vertex_signatures)
        {
            const auto next_color = interner.get_or_create(std::move(signature));
            set_color<G>(next_colors, vertex, next_color);
            changed = changed || next_color != get_color<G>(colors, vertex);
        }

        std::swap(colors, next_colors);
    }

    auto dense_colors = ColorAssignmentList();
    dense_colors.reserve(graph.get_num_vertices());
    for (auto vertex : vertices)
        dense_colors.emplace_back(get_color<G>(colors, vertex), vertex);
    std::sort(dense_colors.begin(), dense_colors.end());

    return Result(std::move(round_summaries), std::move(dense_colors));
}

}  // namespace runir::graphs::color_refinement

#endif
