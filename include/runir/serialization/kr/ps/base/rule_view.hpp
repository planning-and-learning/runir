#ifndef RUNIR_SERIALIZATION_KR_PS_BASE_RULE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_BASE_RULE_VIEW_HPP_

#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/serialization/kr/ps/condition_view.hpp"
#include "runir/serialization/kr/ps/effect_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::base::Rule>, C>>)
{
    ar.field("symbol", [](const auto& value) -> decltype(auto) { return (value.get_symbol()); });
    ar.field("conditions", [](const auto& value) -> decltype(auto) { return (value.get_conditions()); });
    ar.field("effects", [](const auto& value) -> decltype(auto) { return (value.get_effects()); });
}

}

#endif
