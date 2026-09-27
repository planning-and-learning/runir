#ifndef RUNIR_SERIALIZATION_KR_PS_ICP_MODULE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_ICP_MODULE_VIEW_HPP_

#include "runir/kr/ps/icp/module_view.hpp"
#include "runir/serialization/kr/dl/argument_view.hpp"
#include "runir/serialization/kr/dl/register_view.hpp"
#include "runir/serialization/kr/ps/icp/rule_variant_view.hpp"

#include <utility>
#include <vector>
#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::icp::Module>, C>>)
{
    ar.field("symbol", [](const auto& value) -> decltype(auto) { return (value.get_symbol()); });
    ar.field("concept_registers", [](const auto& value) -> decltype(auto) { return (value.template get_registers<runir::kr::dl::ConceptTag>()); });
    ar.field("role_registers", [](const auto& value) -> decltype(auto) { return (value.template get_registers<runir::kr::dl::RoleTag>()); });
    ar.field("concept_features", [](const auto& value) -> decltype(auto) { return (value.template get_features<runir::kr::dl::ConceptTag>()); });
    ar.field("role_features", [](const auto& value) -> decltype(auto) { return (value.template get_features<runir::kr::dl::RoleTag>()); });
    ar.field("boolean_features", [](const auto& value) -> decltype(auto) { return (value.template get_features<runir::kr::ps::dl::BooleanFeature>()); });
    ar.field("numerical_features", [](const auto& value) -> decltype(auto) { return (value.template get_features<runir::kr::ps::dl::NumericalFeature>()); });
    ar.field("query_features", [](const auto& value) -> decltype(auto) { return (value.get_query_features()); });
    ar.field("entry_memory_state", [](const auto& value) -> decltype(auto) { return (value.get_entry_memory_state()); });
    ar.field("memory_states", [](const auto& value) -> decltype(auto) { return (value.get_memory_states()); });
    ar.field("memory_transitions", [](const auto& value) -> decltype(auto) { return (value.get_memory_transitions()); });
    ar.field("reset_pairs",
             [](const auto& value)
             {
                 using FeatureView =
                     decltype(make_view(Index<runir::kr::ps::Feature<runir::kr::IcpFamilyTag, runir::kr::dl::ConceptTag>> {}, value.get_context()));
                 std::vector<std::pair<FeatureView, FeatureView>> pairs;
                 for (const auto& pair : value.get_reset_pairs())
                     pairs.emplace_back(make_view(pair.before, value.get_context()), make_view(pair.after, value.get_context()));
                 return pairs;
             });
}

}

#endif
