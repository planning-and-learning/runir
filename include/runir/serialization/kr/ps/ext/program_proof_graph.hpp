#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/kr/ps/ext/program_proof_graph.hpp"
#include "runir/serialization/graphs/static_graph.hpp"
#include "runir/serialization/kr/ps/ext/execution_view.hpp"
#include "runir/serialization/kr/ps/ext/rule_variant_view.hpp"
#include "tyr/serialization/formalism/binding_view.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<typename Archive, ::tyr::TaskKind Kind>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::ext::ProgramProofVertexLabel<Kind>>)
{
    ar.field("program_state", [](const auto& value) -> decltype(auto) { return (value.program_state); });
    ar.field("is_initial", [](const auto& value) -> decltype(auto) { return (value.is_initial); });
    ar.field("is_goal", [](const auto& value) -> decltype(auto) { return (value.is_goal); });
    ar.field("is_alive", [](const auto& value) -> decltype(auto) { return (value.is_alive); });
    ar.field("is_unsolvable", [](const auto& value) -> decltype(auto) { return (value.is_unsolvable); });
}

template<typename Archive>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::ext::ProgramProofEdgeLabel>)
{
    ar.field("action", [](const auto& value) -> decltype(auto) { return (value.action); });
    ar.field("rule", [](const auto& value) -> decltype(auto) { return (value.rule); });
}

}

#endif
