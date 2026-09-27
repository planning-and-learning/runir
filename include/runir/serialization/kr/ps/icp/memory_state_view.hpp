#ifndef RUNIR_SERIALIZATION_KR_PS_ICP_MEMORY_STATE_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_PS_ICP_MEMORY_STATE_VIEW_HPP_

#include "runir/kr/ps/icp/memory_state_view.hpp"
#include "runir/kr/ps/icp/repository.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<typename Archive, typename C>
void describe_fields(Archive& ar, std::type_identity<View<Index<runir::kr::ps::icp::MemoryState>, C>>)
{
    ar.field("name", [](const auto& value) -> decltype(auto) { return (value.get_name()); });
}

}

#endif
