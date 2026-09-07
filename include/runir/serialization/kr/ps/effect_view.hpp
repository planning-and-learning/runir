#ifndef RUNIR_SERIALIZATION_KR_PS_EFFECT_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EFFECT_VIEW_HPP_

#include "runir/kr/ps/effect_view.hpp"
#include "runir/serialization/kr/ps/base/dl/effect_view.hpp"
#include "runir/serialization/kr/ps/ext/dl/effect_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, runir::kr::FamilyTag Family, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::EffectVariant<Family>>, C>>)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
}

template<typename Archive, runir::kr::FamilyTag Family, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::ConcreteEffectVariant<Family, runir::kr::DlTag>>, C>>)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
}

}

#endif
