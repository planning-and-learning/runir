#ifndef RUNIR_SERIALIZATION_KR_PS_BASE_DL_CONDITION_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_BASE_DL_CONDITION_VIEW_HPP_

#include "runir/kr/ps/base/dl/condition_view.hpp"
#include "runir/serialization/kr/ps/feature_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename FeatureTag, typename ObservationTag, typename C>
struct TypeName<View<Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>>
{
    static std::string get()
    {
        return std::string(runir::kr::BaseFamilyTag::name) + ".DL." + FeatureTag::keyword + "." + ObservationTag::keyword + ".Condition";
    }
};

template<typename FeatureTag, typename ObservationTag, typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("feature", value.get_feature());
    });
}

}

#endif
