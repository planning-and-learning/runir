#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_RULE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_RULE_VIEW_HPP_

#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"
#include "runir/serialization/kr/ps/dl/condition_view.hpp"
#include "runir/serialization/kr/ps/dl/effect_view.hpp"
#include "runir/serialization/kr/ps/feature_view.hpp"
#include "runir/serialization/kr/ps/memory_state_view.hpp"
#include "runir/serialization/kr/ps/module_symbol_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>
#include <yggdrasil/serialization/fields.hpp>

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<urd::index_view<runir::kr::ps::ext::OrderTerm, C>>)
{
    ar.field("direction", [](const auto& value) { return std::string(value.get_direction() == runir::kr::ps::ext::OrderDirection::MIN ? "min" : "max"); });
    ar.variant([](const auto& value) { return value.get_feature(); });
}

// A rule is described by its fields (yggdrasil/serialization/fields.hpp).

}

#endif
