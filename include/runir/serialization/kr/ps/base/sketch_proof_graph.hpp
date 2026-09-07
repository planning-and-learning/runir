#ifndef RUNIR_SERIALIZATION_KR_PS_BASE_SKETCH_PROOF_GRAPH_HPP_
#define RUNIR_SERIALIZATION_KR_PS_BASE_SKETCH_PROOF_GRAPH_HPP_

#include "runir/kr/ps/base/sketch_proof_graph.hpp"
#include "runir/serialization/datasets/state_graph.hpp"
#include "runir/serialization/kr/ps/base/rule_view.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<typename Archive, ::tyr::TaskKind Kind>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::base::SketchProofVertexLabel<Kind>>)
{
    ar.field("state", [](const auto& value) -> decltype(auto) { return (value.state); });
    ar.field("is_initial", [](const auto& value) -> decltype(auto) { return (value.is_initial); });
    ar.field("is_goal", [](const auto& value) -> decltype(auto) { return (value.is_goal); });
    ar.field("is_alive", [](const auto& value) -> decltype(auto) { return (value.is_alive); });
    ar.field("is_unsolvable", [](const auto& value) -> decltype(auto) { return (value.is_unsolvable); });
}

template<typename Archive>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::base::SketchProofEdgeLabel>)
{
    ar.field("transition", [](const auto& value) -> decltype(auto) { return (value.transition); });
    ar.field("rule", [](const auto& value) -> decltype(auto) { return (value.rule); });
}

}

#endif
