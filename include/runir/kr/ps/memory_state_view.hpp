#ifndef RUNIR_KR_PS_MEMORY_STATE_VIEW_HPP_
#define RUNIR_KR_PS_MEMORY_STATE_VIEW_HPP_

#include "runir/kr/ps/memory_state_data.hpp"

#include <tuple>
#include <yggdrasil/formalism/declarations.hpp>

template<runir::kr::ps::ProgramFamilyTag Family, typename V>
struct urd::view_mixin<runir::kr::ps::MemoryState<Family>, V>
{
    const auto& get_name() const noexcept { return static_cast<const V&>(*this).data().name; }
};

#endif
