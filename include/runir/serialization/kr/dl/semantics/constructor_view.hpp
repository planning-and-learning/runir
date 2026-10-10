#ifndef RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_SEMANTICS_CONSTRUCTOR_VIEW_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/constructor_view.hpp"
#include "runir/serialization/kr/dl/query_view.hpp"
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

/// Variant operands of constructors (concept, role or query), e.g. count's argument or distance's
/// vertices and edges, serialize as the alternative they hold.
template<typename Archive, typename... Ts, typename C>
void describe_fields(Archive& ar, std::type_identity<View<cista::offset::variant<Index<Ts>...>, C>>)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value); });
}

}

#endif
