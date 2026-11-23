#include "runir/datasets/formatter.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <tyr/formalism/planning/formatter.hpp>
#include <urd/dot.hpp>
#include <utility>
#include <yggdrasil/formatting/cista_formatters.hpp>

namespace rd = runir::datasets;
namespace tf = tyr::formalism;

template<tyr::TaskKind Kind>
fmt::format_context::iterator fmt::formatter<rd::StateGraph<Kind>, char>::format(const rd::StateGraph<Kind>& value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", urd::to_dot(value, urd::vertex_data, urd::edge_data)); }

fmt::format_context::iterator fmt::formatter<rd::EquivalenceGraph, char>::format(const rd::EquivalenceGraph& value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", urd::to_dot(value, urd::vertex_data, urd::edge_data)); }

fmt::format_context::iterator fmt::formatter<rd::PredicateContext, char>::format(const rd::PredicateContext& value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value == rd::PredicateContext::STATE ? "state" : "goal"); }

template<tf::FactKind T>
fmt::format_context::iterator fmt::formatter<urd::index_view<rd::PredicateColor<T>, rd::ColorRepository>, char>::format(
    const urd::index_view<rd::PredicateColor<T>, rd::ColorRepository>& value,
    fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}:{}[{}]", value.get_context(), value.get_predicate(), value.get_argument_position()); }

fmt::format_context::iterator fmt::formatter<rd::ColorView, char>::format(const rd::ColorView& value, fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}", value.get_values()); }

template<tyr::TaskKind Kind>
fmt::format_context::iterator fmt::formatter<urd::data<rd::StateGraphVertexLabel<Kind>>, char>::format(const urd::data<rd::StateGraphVertexLabel<Kind>>& value,
                                                                                                       fmt::format_context& context) const
{ return fmt::format_to(context.out(), "state={}", value.state.index()); }

template<tyr::TaskKind Kind>
fmt::format_context::iterator
fmt::formatter<urd::data<rd::AnnotatedStateGraphVertexLabel<Kind>>, char>::format(const urd::data<rd::AnnotatedStateGraphVertexLabel<Kind>>& value,
                                                                                  fmt::format_context& context) const
{
    return fmt::format_to(context.out(),
                          "state={}\ngoal_distance={}\ninitial={}\ngoal={}\nalive={}\nunsolvable={}",
                          value.state.index(),
                          value.goal_distance,
                          value.is_initial,
                          value.is_goal,
                          value.is_alive,
                          value.is_unsolvable);
}

fmt::format_context::iterator fmt::formatter<urd::data<rd::StateGraphEdgeLabel>, char>::format(const urd::data<rd::StateGraphEdgeLabel>& value,
                                                                                               fmt::format_context& context) const
{ return fmt::format_to(context.out(), "{}\ncost={}", std::make_pair(value.action, tf::PlanFormatting {}), value.cost); }

fmt::format_context::iterator fmt::formatter<urd::data<rd::EquivalenceVertexLabel>, char>::format(const urd::data<rd::EquivalenceVertexLabel>& value,
                                                                                                  fmt::format_context& context) const
{ return fmt::format_to(context.out(), "state_graph={}\nstate_vertex={}", value.state_graph_index, value.state_vertex_index); }

fmt::format_context::iterator
fmt::formatter<urd::data<rd::AnnotatedEquivalenceVertexLabel>, char>::format(const urd::data<rd::AnnotatedEquivalenceVertexLabel>& value,
                                                                             fmt::format_context& context) const
{
    return fmt::format_to(context.out(),
                          "state_graph={}\nstate_vertex={}\ngoal_distance={}\ninitial={}\ngoal={}\nalive={}\nunsolvable={}",
                          value.state_graph_index,
                          value.state_vertex_index,
                          value.goal_distance,
                          value.is_initial,
                          value.is_goal,
                          value.is_alive,
                          value.is_unsolvable);
}

fmt::format_context::iterator fmt::formatter<urd::data<rd::EquivalenceEdgeLabel>, char>::format(const urd::data<rd::EquivalenceEdgeLabel>& value,
                                                                                                fmt::format_context& context) const
{ return fmt::format_to(context.out(), "state_graph={}\nstate_edge={}", value.state_graph_index, value.state_edge_index); }

template struct fmt::formatter<urd::index_view<rd::PredicateColor<tf::StaticTag>, rd::ColorRepository>, char>;
template struct fmt::formatter<urd::index_view<rd::PredicateColor<tf::FluentTag>, rd::ColorRepository>, char>;
template struct fmt::formatter<urd::data<rd::StateGraphVertexLabel<tyr::GroundTag>>, char>;
template struct fmt::formatter<urd::data<rd::StateGraphVertexLabel<tyr::LiftedTag>>, char>;
template struct fmt::formatter<urd::data<rd::AnnotatedStateGraphVertexLabel<tyr::GroundTag>>, char>;
template struct fmt::formatter<urd::data<rd::AnnotatedStateGraphVertexLabel<tyr::LiftedTag>>, char>;
template struct fmt::formatter<rd::StateGraph<tyr::GroundTag>, char>;
template struct fmt::formatter<rd::StateGraph<tyr::LiftedTag>, char>;
