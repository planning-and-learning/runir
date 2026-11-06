#ifndef RUNIR_KR_PS_EXT_MEMORY_STATE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MEMORY_STATE_VIEW_HPP_

#include "runir/kr/ps/ext/memory_state_data.hpp"

#include <tuple>
#include <yggdrasil/formalism/declarations.hpp>

template<typename V>
struct urd::view_mixin<runir::kr::ps::ext::MemoryState, V>
{
    const auto& get_name() const noexcept { return static_cast<const V&>(*this).data().name; }
};

#endif
