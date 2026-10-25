#ifndef RUNIR_SERIALIZATION_KR_PS_DL_FEATURE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_DL_FEATURE_VIEW_HPP_

#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/serialization/kr/dl/semantics/constructor_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, runir::kr::FamilyTag Family, typename FeatureTag, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>, C>>)
{
    ar.field("symbol", [](const auto& value) -> decltype(auto) { return (value.get_symbol()); });
    ar.field("expression", [](const auto& value) -> decltype(auto) { return (value.get_expression()); });
}

}  // namespace ygg::serialization

#endif
