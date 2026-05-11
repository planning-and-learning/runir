#ifndef RUNIR_GRAPHS_FORMATTER_HPP_
#define RUNIR_GRAPHS_FORMATTER_HPP_

#include "runir/common/formatter.hpp"
#include "runir/graphs/algorithms/color_refinement.hpp"
#include "runir/graphs/algorithms/weisfeiler_leman.hpp"
#include "runir/graphs/backward_static_graph_view.hpp"
#include "runir/graphs/bidirectional_static_graph.hpp"
#include "runir/graphs/dynamic_graph.hpp"
#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"

#include <string>
#include <string_view>
#include <tyr/common/formatter.hpp>

namespace runir::graphs::format
{

inline auto dot_escape(std::string_view text) -> std::string
{
    auto result = std::string {};
    result.reserve(text.size());

    for (const auto ch : text)
    {
        switch (ch)
        {
            case '\\':
                result += "\\\\";
                break;
            case '"':
                result += "\\\"";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                break;
            default:
                result += ch;
                break;
        }
    }

    return result;
}

template<IsGraph G>
auto dot(const G& graph, std::string_view name = "G") -> std::string
{
    auto text = fmt::format("digraph {} {{\n", name);
    text += "  rankdir=LR;\n";

    for (const auto vertex : graph.get_vertex_indices())
    {
        const auto label = fmt::format("{}", graph.get_vertex(vertex).get_property());
        text += fmt::format("  v{} [label=\"{}\"];\n", vertex, dot_escape(label));
    }

    for (const auto edge : graph.get_edge_indices())
    {
        const auto source = graph.get_source(edge);
        const auto target = graph.get_target(edge);
        const auto label = fmt::format("{}", graph.get_edge(edge).get_property());
        text += fmt::format("  v{} -> v{} [label=\"{}\"];\n", source, target, dot_escape(label));
    }

    text += "}\n";
    return text;
}

}  // namespace runir::graphs::format

namespace fmt
{

template<runir::graphs::IsVertex Vertex>
struct formatter<Vertex, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const Vertex& vertex, FormatContext& ctx) const
    {
        const auto text = fmt::format("Vertex(index={}, property={})", vertex.get_index(), vertex.get_property());
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::graphs::IsEdge Edge>
struct formatter<Edge, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const Edge& edge, FormatContext& ctx) const
    {
        const auto text =
            fmt::format("Edge(index={}, source={}, target={}, property={})", edge.get_index(), edge.get_source(), edge.get_target(), edge.get_property());
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::graphs::IsGraph Graph>
struct formatter<Graph, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const Graph& graph, FormatContext& ctx) const
    {
        const auto text = runir::graphs::format::dot(graph);
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<runir::graphs::Property VertexProperty, runir::graphs::Property EdgeProperty>
struct formatter<runir::graphs::BidirectionalStaticGraph<VertexProperty, EdgeProperty>, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::graphs::BidirectionalStaticGraph<VertexProperty, EdgeProperty>& graph, FormatContext& ctx) const
    {
        const auto text = runir::graphs::format::dot(graph.get_forward_graph());
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<typename VertexProperty>
struct formatter<runir::graphs::color_refinement::Certificate<VertexProperty>, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const runir::graphs::color_refinement::Certificate<VertexProperty>& certificate, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "ColorRefinementCertificate(refinement_colors={}, colors={})",
                              certificate.get_refinement_colors(),
                              certificate.get_colors());
    }
};

template<std::size_t K>
struct formatter<runir::graphs::weisfeiler_leman::Signature<K>, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const runir::graphs::weisfeiler_leman::Signature<K>& signature, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}, {})", signature.color, signature.neighbor_colors);
    }
};

template<std::size_t K>
struct formatter<runir::graphs::weisfeiler_leman::Certificate<K>, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const runir::graphs::weisfeiler_leman::Certificate<K>& certificate, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(),
                              "WeisfeilerLeman{}Certificate(refinement_colors={}, colors={})",
                              K,
                              certificate.get_refinement_colors(),
                              certificate.get_colors());
    }
};

}  // namespace fmt

#endif
