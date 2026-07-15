#ifndef RUNIR_DATASETS_FORMATTER_HPP_
#define RUNIR_DATASETS_FORMATTER_HPP_

#include "runir/config.hpp"
#include "runir/datasets/equivalence_graph.hpp"
#include "runir/datasets/object_graph.hpp"
#include "runir/datasets/state_graph.hpp"
#include "runir/graphs/formatter.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iterator>
#include <string>
#include <string_view>
#include <tyr/formalism/planning/formatter.hpp>
#include <variant>

namespace fmt
{

template<>
struct formatter<runir::datasets::StateObjectGraphVertexLabelEntry, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::StateObjectGraphVertexLabelEntry& entry, FormatContext& ctx) const
    {
        const auto text = std::visit([&](const auto& predicate)
                                     { return fmt::format("state:{}/{}[{}]", predicate.get_name(), predicate.get_arity(), entry.argument_position); },
                                     entry.predicate);
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct formatter<runir::datasets::GoalObjectGraphVertexLabelEntry, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::GoalObjectGraphVertexLabelEntry& entry, FormatContext& ctx) const
    {
        const auto text = std::visit([&](const auto& predicate)
                                     { return fmt::format("goal:{}/{}[{}]", predicate.get_name(), predicate.get_arity(), entry.argument_position); },
                                     entry.predicate);
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct formatter<runir::datasets::ObjectGraphVertexLabelEntry, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::ObjectGraphVertexLabelEntry& entry, FormatContext& ctx) const
    {
        const auto text = std::visit([](const auto& alternative) { return fmt::format("{}", alternative); }, entry);
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct formatter<runir::datasets::ObjectGraphVertexLabel, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::ObjectGraphVertexLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("{}", fmt::join(label.labels, "\n"));
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<tyr::planning::TaskKind Kind>
struct formatter<runir::datasets::StateGraphVertexLabel<Kind>, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::StateGraphVertexLabel<Kind>& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("state={}", label.state.get_index());
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<tyr::planning::TaskKind Kind>
struct formatter<runir::datasets::AnnotatedStateGraphVertexLabel<Kind>, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::datasets::AnnotatedStateGraphVertexLabel<Kind>& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("state={}\ngoal_distance={}\ninitial={}\ngoal={}\nalive={}\nunsolvable={}",
                                      label.state.get_index(),
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
        auto text = fmt::format("({}", label.action.get_action().get_name());
        for (size_t i = 0; i < label.action.get_action().get_original_arity(); ++i)
        {
            fmt::format_to(std::back_inserter(text), " {}", label.action.get_row().get_objects()[i].get_name());
        }
        fmt::format_to(std::back_inserter(text), ")\ncost={}", label.cost);
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
