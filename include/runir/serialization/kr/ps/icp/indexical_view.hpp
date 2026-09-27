#ifndef RUNIR_SERIALIZATION_KR_PS_ICP_INDEXICAL_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_ICP_INDEXICAL_VIEW_HPP_

#include "runir/kr/ps/icp/xcondition_view.hpp"
#include "runir/kr/ps/icp/xeffect_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"
#include "runir/serialization/kr/ps/feature_view.hpp"

#include <optional>
#include <string>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename T, typename C>
    requires(std::same_as<T, runir::kr::ps::icp::XCondition> || std::same_as<T, runir::kr::ps::icp::XEffect>)
void describe_fields(Archive& ar, std::type_identity<View<Index<T>, C>>)
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
                 if (::cista::holds_alternative<runir::kr::ps::icp::ArgumentPosition>(value.get_object_reference()))
                     return value.get_object_reference().template as<runir::kr::ps::icp::ArgumentPosition>().value;
                 return std::nullopt;
             });
    ar.field("register",
             [](const auto& value)
             {
                 using RegisterIndex = Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>;
                 using RegisterView = decltype(make_view(RegisterIndex {}, get_repository(value.get_context()).get_dl_repository()));
                 std::optional<RegisterView> result;
                 if (::cista::holds_alternative<RegisterIndex>(value.get_object_reference()))
                     result = make_view(value.get_object_reference().template as<RegisterIndex>(), get_repository(value.get_context()).get_dl_repository());
                 return result;
             });
    ar.field("concept", [](const auto& value) { return value.get_concept_feature(); });
}

}

#endif
