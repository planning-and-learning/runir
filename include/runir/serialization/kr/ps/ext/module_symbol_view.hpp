#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_SYMBOL_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_SYMBOL_VIEW_HPP_

#include "runir/kr/ps/ext/module_symbol_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename C>
struct TypeName<View<Index<runir::kr::ps::ext::ModuleSymbol>, C>>
{
    static std::string get() { return "Ext.ModuleSymbol"; }
};

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::ext::ModuleSymbol>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("name", value.get_name());
    });
}

}

#endif
