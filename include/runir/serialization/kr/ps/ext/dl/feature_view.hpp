#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_DL_FEATURE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_DL_FEATURE_VIEW_HPP_

#include "runir/kr/ps/ext/dl/feature_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/serialization/kr/dl/semantics/constructor_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename FeatureTag, typename C>
struct TypeName<View<Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C>>
{
    static std::string get()
    {
        if constexpr (requires { FeatureTag::keyword; })
            return std::string(runir::kr::ExtFamilyTag::name) + ".DL." + FeatureTag::keyword + ".Feature";
        else
            return std::string(runir::kr::ExtFamilyTag::name) + ".DL." + FeatureTag::name + ".Feature";
    }
};

template<typename Archive, typename FeatureTag, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::ConcreteFeature<runir::kr::ExtFamilyTag, runir::kr::DlTag, FeatureTag>>, C>>)
{
    ar.field("symbol", [](const auto& value) -> decltype(auto) { return (value.get_symbol()); });
    ar.field("expression", [](const auto& value) -> decltype(auto) { return (value.get_expression()); });
}

}

#endif
