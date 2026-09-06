#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_PROGRAM_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_MODULE_PROGRAM_VIEW_HPP_

#include "runir/kr/ps/ext/module_program_view.hpp"
#include "runir/serialization/kr/ps/ext/module_view.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename C>
struct TypeName<View<Index<runir::kr::ps::ext::ModuleProgram>, C>>
{
    static std::string get() { return "Ext.ModuleProgram"; }
};

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::ext::ModuleProgram>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("entry_module", value.get_entry_module());
        ar.field("modules", value.get_modules());
    });
}

}

#endif
