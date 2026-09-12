#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_RULE_VARIANT_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_RULE_VARIANT_VIEW_HPP_

#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/serialization/kr/ps/ext/rule_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>, C>>)
{
    ar.field("symbol", [](const auto& value) -> decltype(auto) { return (value.get_symbol()); });
    ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
}

}

#endif
