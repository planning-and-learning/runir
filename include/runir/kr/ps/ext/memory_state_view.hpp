#ifndef RUNIR_KR_PS_EXT_MEMORY_STATE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MEMORY_STATE_VIEW_HPP_

#include "runir/kr/ps/ext/memory_state_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::ext::MemoryState> C>
class View<Index<runir::kr::ps::ext::MemoryState>, C> : public ygg::IndexViewBase<runir::kr::ps::ext::MemoryState, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::ext::MemoryState, C>::IndexViewBase;

    const auto& get_name() const noexcept { return this->get_data().name; }
};

}  // namespace ygg

#endif
