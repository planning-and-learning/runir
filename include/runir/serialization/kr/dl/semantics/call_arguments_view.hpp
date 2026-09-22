#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CALL_ARGUMENTS_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CALL_ARGUMENTS_VIEW_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/serialization/kr/dl/semantics/denotation_view.hpp"
#include "tyr/serialization/formalism/object_view.hpp"

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<ygg::View<ygg::Index<::runir::kr::dl::semantics::CallArguments>, C>>)
{
    ar.field("concept_arguments", [](const auto& value) -> decltype(auto) { return (value.template get<::runir::kr::dl::ConceptTag>()); });
    ar.field("role_arguments", [](const auto& value) -> decltype(auto) { return (value.template get<::runir::kr::dl::RoleTag>()); });
    ar.field("boolean_arguments", [](const auto& value) -> decltype(auto) { return (value.template get<::runir::kr::dl::BooleanTag>()); });
    ar.field("numerical_arguments", [](const auto& value) -> decltype(auto) { return (value.template get<::runir::kr::dl::NumericalTag>()); });
}

}  // namespace ygg::serialization

#endif
