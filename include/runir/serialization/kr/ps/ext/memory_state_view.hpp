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

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::ext::MemoryState>, C>>)
{
    ar.field("name", [](const auto& value) -> decltype(auto) { return (value.get_name()); });
}

}

#endif
