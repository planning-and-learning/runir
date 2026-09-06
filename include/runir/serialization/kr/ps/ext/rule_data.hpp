#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_RULE_DATA_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_RULE_DATA_HPP_

#include "runir/kr/ps/ext/rule_data.hpp"
#include "runir/serialization/kr/ps/feature_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename C>
struct TypeName<View<runir::kr::ps::ext::CallArgument, C>>
{
    static std::string get() { return "Ext.CallArgument"; }
};

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<runir::kr::ps::ext::CallArgument, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.variant(value);
    });
}

}

#endif
