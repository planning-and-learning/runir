#ifndef RUNIR_SERIALIZATION_KR_PS_BASE_RULE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_BASE_RULE_VIEW_HPP_

#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/serialization/kr/ps/condition_view.hpp"
#include "runir/serialization/kr/ps/effect_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename C>
struct TypeName<View<Index<runir::kr::ps::base::Rule>, C>>
{
    static std::string get() { return "Base.Rule"; }
};

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::base::Rule>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("symbol", value.get_symbol());
        ar.field("conditions", value.get_conditions());
        ar.field("effects", value.get_effects());
    });
}

}

#endif
