#ifndef RUNIR_SERIALIZATION_DATASETS_STATE_GRAPH_HPP_
#define RUNIR_SERIALIZATION_DATASETS_STATE_GRAPH_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/serialization/graphs/bidirectional_static_graph.hpp"
#include "tyr/serialization/planning/state_view.hpp"
#include "tyr/serialization/formalism/binding_view.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<typename Archive, ::tyr::TaskKind Kind>
void describe_fields(Archive& ar, std::type_identity<::runir::datasets::StateGraphVertexLabel<Kind>>)
{
    ar.field("state", [](const auto& value) -> decltype(auto) { return (value.state); });
}

template<typename Archive, ::tyr::TaskKind Kind>
void describe_fields(Archive& ar, std::type_identity<::runir::datasets::AnnotatedStateGraphVertexLabel<Kind>>)
{
    ar.field("state", [](const auto& value) -> decltype(auto) { return (value.state); });
    ar.field("goal_distance", [](const auto& value) -> decltype(auto) { return (value.goal_distance); });
    ar.field("is_initial", [](const auto& value) -> decltype(auto) { return (value.is_initial); });
    ar.field("is_goal", [](const auto& value) -> decltype(auto) { return (value.is_goal); });
    ar.field("is_alive", [](const auto& value) -> decltype(auto) { return (value.is_alive); });
    ar.field("is_unsolvable", [](const auto& value) -> decltype(auto) { return (value.is_unsolvable); });
}

template<typename Archive>
void describe_fields(Archive& ar, std::type_identity<::runir::datasets::StateGraphEdgeLabel>)
{
    ar.field("action", [](const auto& value) -> decltype(auto) { return (value.action); });
    ar.field("cost", [](const auto& value) -> decltype(auto) { return (value.cost); });
}

}

#endif
