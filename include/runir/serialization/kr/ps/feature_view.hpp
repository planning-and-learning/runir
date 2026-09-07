#ifndef RUNIR_SERIALIZATION_KR_PS_FEATURE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_FEATURE_VIEW_HPP_

#include "runir/kr/ps/feature_view.hpp"
#include "runir/kr/ps/repository.hpp"
#include "runir/serialization/kr/ps/base/dl/feature_view.hpp"
#include "runir/serialization/kr/ps/ext/dl/feature_view.hpp"
#include "runir/serialization/kr/uns/dl/feature_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<runir::kr::FamilyTag Family, typename FeatureTag, typename C>
struct TypeName<View<Index<runir::kr::ps::Feature<Family, FeatureTag>>, C>>
{
    static std::string get()
    {
        if constexpr (requires { FeatureTag::keyword; })
            return std::string(Family::name) + "." + FeatureTag::keyword + ".Feature";
        else
            return std::string(Family::name) + "." + FeatureTag::name + ".Feature";
    }
};

template<typename Archive, runir::kr::FamilyTag Family, typename FeatureTag, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::Feature<Family, FeatureTag>>, C>>)
{
    ar.variant([](const auto& value) -> decltype(auto) { return (value.get_variant()); });
}

}

#endif
