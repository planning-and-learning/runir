#ifndef RUNIR_SERIALIZATION_KR_PS_BASE_SKETCH_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_BASE_SKETCH_VIEW_HPP_

#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/serialization/kr/ps/base/rule_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename C>
struct TypeName<View<Index<runir::kr::ps::base::Sketch>, C>>
{
    static std::string get() { return "Base.Sketch"; }
};

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::base::Sketch>, C>>)
{
    ar.field("boolean_features", [](const auto& value) -> decltype(auto) { return (value.template get_features<runir::kr::ps::dl::BooleanFeature>()); });
    ar.field("numerical_features", [](const auto& value) -> decltype(auto) { return (value.template get_features<runir::kr::ps::dl::NumericalFeature>()); });
    ar.field("rules", [](const auto& value) -> decltype(auto) { return (value.get_rules()); });
}

}

#endif
