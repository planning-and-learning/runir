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

template<IsGraph G>
using Coloring = std::conditional_t<IsDenseGraph<G>, ColorList, tyr::UnorderedMap<VertexIndex, Color>>;

template<typename VertexProperty>
class Certificate
{
public:
    using Signature = std::pair<Color, ColorList>;
    using SignatureList = std::vector<std::pair<Signature, Color>>;

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

template<typename Signature>
class SignatureInterner
{
private:
    std::vector<std::pair<Signature, Color>> m_data;
    tyr::UnorderedMap<Signature, Color> m_index;

public:
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

    auto colors = make_coloring_storage(graph);

    for (auto vertex : graph.get_vertex_indices())
    {
        const auto color = static_cast<Color>(graph.get_vertex(vertex).get_property_index());
        set_color<G>(colors, vertex, color);
    }

    auto refinement_colors = typename Result::SignatureList();
    auto changed = true;
    while (changed)
    {
        changed = false;

        auto interner = SignatureInterner<Signature>();
        auto next_colors = make_coloring_storage(graph);
        for (auto vertex : graph.get_vertex_indices())
        {
            auto neighbor_colors = ColorList();
            neighbor_colors.reserve(graph.get_out_degree(vertex));
            for (auto edge : graph.get_out_edge_indices(vertex))
                neighbor_colors.push_back(get_color<G>(colors, graph.get_target(edge)));
            std::sort(neighbor_colors.begin(), neighbor_colors.end());

            auto signature = Signature(get_color<G>(colors, vertex), std::move(neighbor_colors));
            const auto next_color = interner.get_or_create(std::move(signature));
            set_color<G>(next_colors, vertex, next_color);
            changed = changed || next_color != get_color<G>(colors, vertex);
        }

        refinement_colors = std::move(interner).release();
        colors = std::move(next_colors);
    }

    auto dense_colors = ColorList();
    dense_colors.reserve(graph.get_num_vertices());
    for (auto vertex : to_dense_vertex_order(graph))
        dense_colors.push_back(get_color<G>(colors, vertex));

    return Result(std::move(refinement_colors), std::move(dense_colors));
}

}  // namespace runir::graphs::color_refinement

#endif
