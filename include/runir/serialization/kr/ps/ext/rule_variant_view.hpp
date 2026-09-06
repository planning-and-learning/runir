#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_RULE_VARIANT_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_RULE_VARIANT_VIEW_HPP_

#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/serialization/kr/ps/ext/rule_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename C>
struct TypeName<View<Index<runir::kr::ps::ext::RuleVariant>, C>>
{
    static std::string get() { return "Ext.Rule"; }
};

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::ext::RuleVariant>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("symbol", value.get_symbol());
        ar.variant(value.get_variant());
    });
}

}

#endif
