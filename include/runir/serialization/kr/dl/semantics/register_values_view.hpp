#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_REGISTER_VALUES_VIEW_HPP_

#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/serialization/kr/dl/semantics/denotation_view.hpp"
#include "tyr/serialization/formalism/object_view.hpp"

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<ygg::View<ygg::Index<::runir::kr::dl::semantics::RegisterValues>, C>>)
{
    ar.field("concept_values", [](const auto& value) -> decltype(auto) { return (value.get_concept_values()); });
    ar.field("role_values", [](const auto& value) -> decltype(auto) { return (value.get_role_values()); });
}

}  // namespace ygg::serialization

#endif
