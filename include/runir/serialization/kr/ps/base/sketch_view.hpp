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

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::base::Sketch>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("boolean_features", value.template get_features<runir::kr::ps::dl::BooleanFeature>());
        ar.field("numerical_features", value.template get_features<runir::kr::ps::dl::NumericalFeature>());
        ar.field("rules", value.get_rules());
    });
}

}

#endif
