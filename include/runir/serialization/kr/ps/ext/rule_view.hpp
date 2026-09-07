#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_RULE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_RULE_VIEW_HPP_

#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"
#include "runir/serialization/kr/ps/condition_view.hpp"
#include "runir/serialization/kr/ps/effect_view.hpp"
#include "runir/serialization/kr/ps/ext/memory_state_view.hpp"
#include "runir/serialization/kr/ps/ext/module_symbol_view.hpp"
#include "runir/serialization/kr/ps/ext/rule_data.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, runir::kr::ps::ext::RuleKind Kind, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::ext::Rule<Kind>>, C>>)
{
    using Value = View<Index<runir::kr::ps::ext::Rule<Kind>>, C>;

    ar.field("source", [](const auto& value) -> decltype(auto) { return (value.get_source()); });
    ar.field("target", [](const auto& value) -> decltype(auto) { return (value.get_target()); });
    ar.field("conditions", [](const auto& value) -> decltype(auto) { return (value.get_conditions()); });
    if constexpr (requires(const Value& value) { value.get_effects(); })
        ar.field("effects", [](const auto& value) -> decltype(auto) { return (value.get_effects()); });
    if constexpr (requires(const Value& value) { value.get_feature(); })
    {
        ar.field("feature", [](const auto& value) -> decltype(auto) { return (value.get_feature()); });
        ar.field("register", [](const auto& value) -> decltype(auto) { return (value.get_register()); });
    }
    if constexpr (requires(const Value& value) { value.get_action_name(); })
    {
        ar.field("action_name", [](const auto& value) -> decltype(auto) { return (value.get_action_name()); });
        ar.field("arguments", [](const auto& value) -> decltype(auto) { return (value.get_action_arguments()); });
    }
    if constexpr (requires(const Value& value) { value.get_callee(); })
    {
        ar.field("callee", [](const auto& value) -> decltype(auto) { return (value.get_callee()); });
        ar.field("arguments", [](const auto& value) -> decltype(auto) { return (make_view(value.get_data().arguments, value.get_context())); });
    }
}

}

#endif
