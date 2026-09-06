#ifndef RUNIR_SERIALIZATION_KR_UNS_DL_FEATURE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_UNS_DL_FEATURE_VIEW_HPP_

#include "runir/kr/uns/dl/feature_view.hpp"
#include "runir/kr/uns/repository.hpp"
#include "runir/serialization/kr/dl/semantics/constructor_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename C>
struct TypeName<View<Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>, C>>
{
    static std::string get() { return std::string(runir::kr::UnsFamilyTag::name) + ".DL." + runir::kr::ps::dl::BooleanFeature::keyword + ".Feature"; }
};

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>, C>& value,
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
