#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_PROGRAM_PROOF_GRAPH_HPP_

#include "runir/kr/ps/ext/module_program_proof_graph.hpp"
#include "runir/serialization/graphs/static_graph.hpp"
#include "runir/serialization/kr/ps/ext/execution_view.hpp"
#include "runir/serialization/kr/ps/ext/rule_variant_view.hpp"
#include "tyr/serialization/formalism/binding_view.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<::tyr::TaskKind Kind>
struct TypeName<::runir::kr::ps::ext::ModuleProgramProofVertexLabel<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "ModuleProgramProofVertexLabel"; }
};

template<typename Archive, ::tyr::TaskKind Kind>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::ext::ModuleProgramProofVertexLabel<Kind>>)
{
    ar.field("execution_state", [](const auto& value) -> decltype(auto) { return (value.execution_state); });
    ar.field("is_initial", [](const auto& value) -> decltype(auto) { return (value.is_initial); });
    ar.field("is_goal", [](const auto& value) -> decltype(auto) { return (value.is_goal); });
    ar.field("is_alive", [](const auto& value) -> decltype(auto) { return (value.is_alive); });
    ar.field("is_unsolvable", [](const auto& value) -> decltype(auto) { return (value.is_unsolvable); });
}

template<>
struct TypeName<::runir::kr::ps::ext::ModuleProgramProofStateTransition>
{
    static std::string get() { return "ModuleProgramProofStateTransition"; }
};

template<typename Archive>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::ext::ModuleProgramProofStateTransition>)
{
    ar.field("action", [](const auto& value) -> decltype(auto) { return (value.action); });
    ar.field("cost", [](const auto& value) -> decltype(auto) { return (value.cost); });
}

template<>
struct TypeName<::runir::kr::ps::ext::ModuleProgramProofEdgeLabel>
{
    static std::string get() { return "ModuleProgramProofEdgeLabel"; }
};

template<typename Archive>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::ext::ModuleProgramProofEdgeLabel>)
{
    ar.field("state_transition", [](const auto& value) -> decltype(auto) { return (value.state_transition); });
    ar.field("rule", [](const auto& value) -> decltype(auto) { return (value.rule); });
}

}

#endif
