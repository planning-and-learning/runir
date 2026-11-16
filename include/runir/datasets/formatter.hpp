#ifndef RUNIR_DATASETS_FORMATTER_HPP_
#define RUNIR_DATASETS_FORMATTER_HPP_

#include "runir/datasets/equivalence_graph.hpp"
#include "runir/datasets/object_graph.hpp"
#include "runir/datasets/state_graph.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <string_view>
#include <tyr/formalism/planning/formatter.hpp>
#include <urd/dot.hpp>
#include <utility>
#include <variant>
#include <yggdrasil/formatting/cista_formatters.hpp>

namespace runir::datasets
{

/// A predicate context as fmt writes it (format_as).
inline const char* format_as(PredicateContext context) noexcept { return context == PredicateContext::STATE ? "state" : "goal"; }

}  // namespace runir::datasets

namespace fmt
{

/// A runir graph prints as DOT (urd::to_dot: each vertex and edge labelled by its label).
template<runir::labeled_graph G>
struct formatter<G, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const G& graph, FormatContext& ctx) const
    { return formatter<std::string_view>::format(urd::to_dot(graph, urd::vertex_data, urd::edge_data), ctx); }
};

template<::tyr::formalism::FactKind T>
struct formatter<urd::index_view<runir::datasets::PredicateColor<T>, runir::datasets::ColorRepository>, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const urd::index_view<runir::datasets::PredicateColor<T>, runir::datasets::ColorRepository>& color, FormatContext& ctx) const
    {
        const auto predicate = color.get_predicate();
        const auto context = format_as(color.get_context());
        const auto text = fmt::format("{}:{}[{}]", context, predicate, color.get_argument_position());
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct formatter<runir::datasets::ColorView, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::ColorView& color, FormatContext& ctx) const
    {
        const auto text = fmt::format("{}", color.get_values());
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<tyr::TaskKind Kind>
struct formatter<runir::datasets::StateGraphVertexLabel<Kind>, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::StateGraphVertexLabel<Kind>& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("state={}", label.state.index());
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<tyr::TaskKind Kind>
struct formatter<runir::datasets::AnnotatedStateGraphVertexLabel<Kind>, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::AnnotatedStateGraphVertexLabel<Kind>& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("state={}\ngoal_distance={}\ninitial={}\ngoal={}\nalive={}\nunsolvable={}",
                                      label.state.index(),
                                      label.goal_distance,
                                      label.is_initial,
                                      label.is_goal,
                                      label.is_alive,
                                      label.is_unsolvable);
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct formatter<runir::datasets::StateGraphEdgeLabel, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::StateGraphEdgeLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("{}\ncost={}", std::make_pair(label.action, tyr::formalism::planning::PlanFormatting {}), label.cost);
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct formatter<runir::datasets::EquivalenceVertexLabel, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::EquivalenceVertexLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("state_graph={}\nstate_vertex={}", label.state_graph_index, label.state_vertex_index);
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct formatter<runir::datasets::AnnotatedEquivalenceVertexLabel, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::AnnotatedEquivalenceVertexLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("state_graph={}\nstate_vertex={}\ngoal_distance={}\ninitial={}\ngoal={}\nalive={}\nunsolvable={}",
                                      label.state_graph_index,
                                      label.state_vertex_index,
                                      label.goal_distance,
                                      label.is_initial,
                                      label.is_goal,
                                      label.is_alive,
                                      label.is_unsolvable);
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct formatter<runir::datasets::EquivalenceEdgeLabel, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::EquivalenceEdgeLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("state_graph={}\nstate_edge={}", label.state_graph_index, label.state_edge_index);
        return formatter<std::string_view>::format(text, ctx);
    }
};

}  // namespace fmt

#endif
