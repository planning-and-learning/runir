#ifndef RUNIR_GRAPHS_FORMATTER_HPP_
#define RUNIR_GRAPHS_FORMATTER_HPP_

#include "runir/common/formatter.hpp"
#include "runir/graphs/algorithms/color_refinement.hpp"
#include "runir/graphs/algorithms/weisfeiler_leman.hpp"

namespace fmt
{

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
