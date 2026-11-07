#ifndef RUNIR_SERIALIZATION_KR_PS_RULE_VARIANT_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_RULE_VARIANT_VIEW_HPP_

#include "runir/kr/ps/rule_variant_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

/// The variant's alternatives are described by the family's rule_view serialization.
template<typename Archive, runir::kr::ps::ProgramFamilyTag Family, typename C>
void describe_fields(Archive& ar, std::type_identity<urd::index_view<runir::kr::ps::Rule<Family>, C>>)
{
    ar.field("symbol", [](const auto& value) -> decltype(auto) { return (value.get_symbol()); });
    ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
}

}

#endif
