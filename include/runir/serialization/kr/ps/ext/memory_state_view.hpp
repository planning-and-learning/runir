#ifndef RUNIR_SERIALIZATION_KR_PS_EXT_MEMORY_STATE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_EXT_MEMORY_STATE_VIEW_HPP_

#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename C>
struct TypeName<View<Index<runir::kr::ps::ext::MemoryState>, C>>
{
    static std::string get() { return "Ext.MemoryState"; }
};

template<typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::ps::ext::MemoryState>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("name", value.get_name());
    });
}

}

#endif
