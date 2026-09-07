#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/serialization/kr/dl/semantics/boolean_view.hpp"
#include "runir/serialization/kr/dl/semantics/concept_view.hpp"
#include "runir/serialization/kr/dl/semantics/numerical_view.hpp"
#include "runir/serialization/kr/dl/semantics/role_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::dl::Constructor<Family, Category>>, C>>)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
}

template<typename Archive, runir::kr::dl::FamilyTag Family, typename C>
void describe_fields(Archive& ar,
                     std::type_identity<View<cista::offset::variant<Index<runir::kr::dl::Constructor<Family, runir::kr::dl::ConceptTag>>,
                                                                    Index<runir::kr::dl::Constructor<Family, runir::kr::dl::RoleTag>>>, C>>)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value); });
}

}

#endif
