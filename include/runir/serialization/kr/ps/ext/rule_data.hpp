#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_RULE_DATA_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_RULE_DATA_HPP_

#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/serialization/kr/ps/feature_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<runir::kr::ps::ext::CallArgument, C>>)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value); });
}

}

#endif
