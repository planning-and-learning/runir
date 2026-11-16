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

/// An indexical condition or effect T: its operation, its object (an argument position or a register slot) and its
/// concept.
template<typename T, typename Archive>
void describe_indexical_fields(Archive& ar)
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
                 const auto reference = value.get_object();
                 if (const auto* position = std::get_if<runir::kr::ps::icp::ArgumentPosition>(&reference))
                     return position->value;
                 return std::nullopt;
             });
    ar.field("slot",
             [](const auto& value)
             {
                 const auto reference = value.get_object();
                 using RegisterView = std::variant_alternative_t<1, std::remove_const_t<decltype(reference)>>;
                 const auto* reg = std::get_if<RegisterView>(&reference);
                 return reg ? std::optional<RegisterView>(*reg) : std::nullopt;
             });
    ar.field("concept", [](const auto& value) { return value.get_feature(); });
}

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<urd::index_view<runir::kr::ps::icp::XCondition, C>> /*type*/)
{ describe_indexical_fields<runir::kr::ps::icp::XCondition>(ar); }

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<urd::index_view<runir::kr::ps::icp::XEffect, C>> /*type*/)
{ describe_indexical_fields<runir::kr::ps::icp::XEffect>(ar); }

}

#endif
