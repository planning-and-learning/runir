#ifndef RUNIR_SERIALIZATION_KR_PS_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_SERIALIZATION_KR_PS_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/kr/ps/program_proof_graph.hpp"
#include "runir/serialization/graphs/static_graph.hpp"
#include "runir/serialization/kr/ps/rule_variant_view.hpp"
#include "tyr/serialization/formalism/planning/planning.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

/// The program state is described by the family's execution_view serialization.
template<typename Archive, ::runir::kr::ps::ProgramFamilyTag Family, ::tyr::TaskKind Kind>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::ProgramProofVertexLabel<Family, Kind>>)
{
    ar.field("program_state", [](const auto& value) -> decltype(auto) { return (value.program_state); });
    ar.field("is_initial", [](const auto& value) -> decltype(auto) { return (value.is_initial); });
    ar.field("is_goal", [](const auto& value) -> decltype(auto) { return (value.is_goal); });
    ar.field("is_alive", [](const auto& value) -> decltype(auto) { return (value.is_alive); });
    ar.field("is_unsolvable", [](const auto& value) -> decltype(auto) { return (value.is_unsolvable); });
}

template<typename Archive, ::runir::kr::ps::ProgramFamilyTag Family>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::ProgramProofEdgeLabel<Family>>)
{
    ar.field("action", [](const auto& value) -> decltype(auto) { return (value.action); });
    ar.field("rule", [](const auto& value) -> decltype(auto) { return (value.rule); });
}

}

#endif
