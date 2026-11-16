#ifndef RUNIR_SERIALIZATION_KR_PS_ICP_INDEXICAL_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_ICP_INDEXICAL_VIEW_HPP_

#include "runir/kr/ps/icp/xcondition_view.hpp"
#include "runir/kr/ps/icp/xeffect_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"
#include "runir/serialization/kr/ps/feature_view.hpp"

#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename T, typename C>
    requires(std::same_as<T, runir::kr::ps::icp::XCondition> || std::same_as<T, runir::kr::ps::icp::XEffect>)
void describe_fields(Archive& ar, std::type_identity<urd::index_view<T, C>>)
{
    ar.field("operation",
             [](const auto& value)
             {
                 if constexpr (std::same_as<T, runir::kr::ps::icp::XCondition>)
                     return std::string(value.get_operation() == runir::kr::ps::icp::ConditionOperation::BELONGS ? "belongs" : "not-belongs");
                 else
                     return std::string(value.get_operation() == runir::kr::ps::icp::EffectOperation::ENTER ? "enter" : "exit");
             });
    ar.field("argument_position",
             [](const auto& value) -> std::optional<ygg::uint_t>
             {
                 const auto reference = value.get_object_reference();
                 if (const auto* position = std::get_if<runir::kr::ps::icp::ArgumentPosition>(&reference))
                     return position->value;
                 return std::nullopt;
             });
    ar.field("slot",
             [](const auto& value)
             {
                 const auto reference = value.get_object_reference();
                 using RegisterView = std::variant_alternative_t<1, std::remove_const_t<decltype(reference)>>;
                 const auto* reg = std::get_if<RegisterView>(&reference);
                 return reg ? std::optional<RegisterView>(*reg) : std::nullopt;
             });
    ar.field("concept", [](const auto& value) { return value.get_concept_feature(); });
}

}

#endif
