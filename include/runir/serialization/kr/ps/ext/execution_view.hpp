#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_EXECUTION_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_EXECUTION_VIEW_HPP_

#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/serialization/kr/dl/semantics/denotation_view.hpp"
#include "runir/serialization/kr/ps/ext/execution_declarations.hpp"
#include "runir/serialization/kr/ps/ext/module_program_view.hpp"
#include "tyr/planning/state_repository.hpp"
#include "tyr/serialization/formalism/object_view.hpp"
#include "tyr/serialization/planning/state_view.hpp"
#include "yggdrasil/serialization/dictionaries.hpp"

namespace ygg::serialization
{

template<::tyr::TaskKind Kind>
struct TypeName<::runir::kr::ps::ext::RegisterValuesView<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "RegisterValues"; }
};

template<typename Archive, ::tyr::TaskKind Kind>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::ext::RegisterValuesView<Kind>>)
{
    ar.field("concept_values", [](const auto& value) -> decltype(auto) { return (value.get_concept_values()); });
    ar.field("role_values", [](const auto& value) -> decltype(auto) { return (value.get_role_values()); });
}

template<::tyr::TaskKind Kind>
struct TypeName<::runir::kr::ps::ext::CallArgumentsView<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "CallArguments"; }
};

template<typename Archive, ::tyr::TaskKind Kind>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::ext::CallArgumentsView<Kind>>)
{
    ar.field("concept_arguments", [](const auto& value) -> decltype(auto) { return (value.template get<::runir::kr::dl::ConceptTag>()); });
    ar.field("role_arguments", [](const auto& value) -> decltype(auto) { return (value.template get<::runir::kr::dl::RoleTag>()); });
    ar.field("boolean_arguments", [](const auto& value) -> decltype(auto) { return (value.template get<::runir::kr::dl::BooleanTag>()); });
    ar.field("numerical_arguments", [](const auto& value) -> decltype(auto) { return (value.template get<::runir::kr::dl::NumericalTag>()); });
}

template<::tyr::TaskKind Kind>
struct TypeName<::runir::kr::ps::ext::CallStackView<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "CallStack"; }
};

template<typename Archive, ::tyr::TaskKind Kind>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::ext::CallStackView<Kind>>)
{
    ar.field("module", [](const auto& value) -> decltype(auto) { return (value.get_module()); });
    ar.field("memory_state", [](const auto& value) -> decltype(auto) { return (value.get_memory_state()); });
    ar.field("registers", [](const auto& value) -> decltype(auto) { return (value.get_registers()); });
    ar.field("arguments", [](const auto& value) -> decltype(auto) { return (value.get_arguments()); });
    ar.field("caller", [](const auto& value) -> decltype(auto) { return (value.get_caller()); });
}

template<::tyr::TaskKind Kind>
struct TypeName<::runir::kr::ps::ext::ExecutionStateView<Kind>>
{
    static std::string get() { return std::string(Kind::name) + "ExecutionState"; }
};

template<typename Archive, ::tyr::TaskKind Kind>
void describe_fields(Archive& ar, std::type_identity<::runir::kr::ps::ext::ExecutionStateView<Kind>>)
{
    ar.field("state", [](const auto& value) -> decltype(auto) { return (value.get_state()); });
    ar.field("program", [](const auto& value) -> decltype(auto) { return (value.get_program()); });
    ar.field("phase", [](const auto& value) -> decltype(auto) { return (value.get_phase()); });
    ar.field("call_stack", [](const auto& value) -> decltype(auto) { return (value.get_call_stack()); });
}


}

#endif
