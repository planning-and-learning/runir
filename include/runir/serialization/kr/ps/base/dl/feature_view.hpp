#ifndef RUNIR_SERIALIZATION_KR_PS_BASE_DL_FEATURE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_BASE_DL_FEATURE_VIEW_HPP_

#include "runir/kr/ps/base/dl/feature_view.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/serialization/kr/dl/semantics/constructor_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename FeatureTag, typename C>
struct TypeName<View<Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C>>
{
    static std::string get() { return std::string(runir::kr::BaseFamilyTag::name) + ".DL." + FeatureTag::keyword + ".Feature"; }
};

template<typename FeatureTag, typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("symbol", value.get_symbol());
        ar.field("expression", value.get_expression());
    });
}

}

#endif
