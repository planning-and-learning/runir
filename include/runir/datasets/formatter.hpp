#ifndef RUNIR_DATASETS_FORMATTER_HPP_
#define RUNIR_DATASETS_FORMATTER_HPP_

#include "runir/datasets/equivalence_graph.hpp"
#include "runir/datasets/object_graph.hpp"
#include "runir/datasets/state_graph.hpp"
#include "runir/formatter.hpp"

#include <fmt/format.h>

// The datasets' graphs, colors and graph labels: each has an fmt::formatter whose format is defined in
// src/datasets/formatter.cpp, and explicitly instantiated there for each fact or task kind.

namespace fmt
{

/// A state graph, as DOT (urd::to_dot): each vertex and edge labelled by its label.
template<tyr::TaskKind Kind>
struct formatter<runir::datasets::StateGraph<Kind>, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::datasets::StateGraph<Kind>& value, fmt::format_context& context) const;
};

/// An equivalence graph, as DOT (urd::to_dot): each vertex and edge labelled by its label.
template<>
struct formatter<runir::datasets::EquivalenceGraph, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::datasets::EquivalenceGraph& value, fmt::format_context& context) const;
};

/// A predicate context: state or goal.
template<>
struct formatter<runir::datasets::PredicateContext, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::datasets::PredicateContext& value, fmt::format_context& context) const;
};

/// A predicate color: context:predicate[argument position].
template<::tyr::formalism::FactKind T>
struct formatter<urd::index_view<runir::datasets::PredicateColor<T>, runir::datasets::ColorRepository>, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const urd::index_view<runir::datasets::PredicateColor<T>, runir::datasets::ColorRepository>& value,
                                         fmt::format_context& context) const;
};

/// A color: its values as a list.
template<>
struct formatter<runir::datasets::ColorView, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::datasets::ColorView& value, fmt::format_context& context) const;
};

/// A state graph's vertex: state=index.
template<tyr::TaskKind Kind>
struct formatter<runir::datasets::StateGraphVertexLabel<Kind>, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::datasets::StateGraphVertexLabel<Kind>& value, fmt::format_context& context) const;
};

/// An annotated state graph's vertex: state, goal_distance, initial, goal, alive and unsolvable, one per line.
template<tyr::TaskKind Kind>
struct formatter<runir::datasets::AnnotatedStateGraphVertexLabel<Kind>, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::datasets::AnnotatedStateGraphVertexLabel<Kind>& value, fmt::format_context& context) const;
};

/// A state graph's edge: its action as a plan lists it, then cost=c on the next line.
template<>
struct formatter<runir::datasets::StateGraphEdgeLabel, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::datasets::StateGraphEdgeLabel& value, fmt::format_context& context) const;
};

/// An equivalence graph's vertex: state_graph and state_vertex, one per line.
template<>
struct formatter<runir::datasets::EquivalenceVertexLabel, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::datasets::EquivalenceVertexLabel& value, fmt::format_context& context) const;
};

/// An annotated equivalence graph's vertex: state_graph, state_vertex, goal_distance, initial, goal, alive and unsolvable, one per line.
template<>
struct formatter<runir::datasets::AnnotatedEquivalenceVertexLabel, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::datasets::AnnotatedEquivalenceVertexLabel& value, fmt::format_context& context) const;
};

/// An equivalence graph's edge: state_graph and state_edge, one per line.
template<>
struct formatter<runir::datasets::EquivalenceEdgeLabel, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::datasets::EquivalenceEdgeLabel& value, fmt::format_context& context) const;
};

}  // namespace fmt

#endif
