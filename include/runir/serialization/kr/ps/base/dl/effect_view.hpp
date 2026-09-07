#ifndef RUNIR_SERIALIZATION_KR_PS_BASE_DL_EFFECT_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_BASE_DL_EFFECT_VIEW_HPP_

#include "runir/kr/ps/base/dl/effect_view.hpp"
#include "runir/serialization/kr/ps/feature_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename FeatureTag, typename ObservationTag, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>>)
{
    ar.field("feature", [](const auto& value) -> decltype(auto) { return (value.get_feature()); });
}

}

#endif
