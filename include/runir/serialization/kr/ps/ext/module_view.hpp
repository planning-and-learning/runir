#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_VIEW_HPP_

#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/serialization/kr/dl/argument_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"
#include "runir/serialization/kr/ps/ext/rule_variant_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::ext::Module>, C>>)
{
    ar.field("symbol", [](const auto& value) -> decltype(auto) { return (value.get_symbol()); });
    ar.field("concept_arguments", [](const auto& value) -> decltype(auto) { return (value.template get_arguments<runir::kr::dl::ConceptTag>()); });
    ar.field("role_arguments", [](const auto& value) -> decltype(auto) { return (value.template get_arguments<runir::kr::dl::RoleTag>()); });
    ar.field("boolean_arguments", [](const auto& value) -> decltype(auto) { return (value.template get_arguments<runir::kr::dl::BooleanTag>()); });
    ar.field("numerical_arguments", [](const auto& value) -> decltype(auto) { return (value.template get_arguments<runir::kr::dl::NumericalTag>()); });
    ar.field("concept_registers", [](const auto& value) -> decltype(auto) { return (value.template get_registers<runir::kr::dl::ConceptTag>()); });
    ar.field("role_registers", [](const auto& value) -> decltype(auto) { return (value.template get_registers<runir::kr::dl::RoleTag>()); });
    ar.field("concept_features", [](const auto& value) -> decltype(auto) { return (value.template get_features<runir::kr::dl::ConceptTag>()); });
    ar.field("role_features", [](const auto& value) -> decltype(auto) { return (value.template get_features<runir::kr::dl::RoleTag>()); });
    ar.field("boolean_features", [](const auto& value) -> decltype(auto) { return (value.template get_features<runir::kr::ps::dl::BooleanFeature>()); });
    ar.field("numerical_features", [](const auto& value) -> decltype(auto) { return (value.template get_features<runir::kr::ps::dl::NumericalFeature>()); });
    ar.field("entry_memory_state", [](const auto& value) -> decltype(auto) { return (value.get_entry_memory_state()); });
    ar.field("memory_states", [](const auto& value) -> decltype(auto) { return (value.get_memory_states()); });
    ar.field("memory_transitions", [](const auto& value) -> decltype(auto) { return (value.get_memory_transitions()); });
}

}

#endif
