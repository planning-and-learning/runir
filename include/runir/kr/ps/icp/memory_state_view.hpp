#ifndef RUNIR_KR_PS_ICP_MEMORY_STATE_VIEW_HPP_
#define RUNIR_KR_PS_ICP_MEMORY_STATE_VIEW_HPP_

#include "runir/kr/ps/icp/memory_state_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<formalism::SymbolContextFor<runir::kr::ps::icp::MemoryState> C>
class View<Index<runir::kr::ps::icp::MemoryState>, C> : public formalism::detail::View<Index<runir::kr::ps::icp::MemoryState>, C>
{
public:
    View(Index<runir::kr::ps::icp::MemoryState> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::ps::icp::MemoryState>, C>(handle, context)
    {
    }

    const auto& get_name() const noexcept { return this->get_data().name; }
};

}  // namespace ygg

#endif
