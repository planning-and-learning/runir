#ifndef RUNIR_KR_PS_EXT_EXECUTION_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_CANONICALIZATION_HPP_

#include "runir/kr/dl/semantics/canonicalization.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"

namespace runir::kr::ps::ext
{

inline bool is_canonical(const ygg::Data<CallStack>&) noexcept { return true; }

template<tyr::TaskKind Kind>
bool is_canonical(const ygg::Data<ExecutionState<Kind>>&) noexcept
{
    return true;
}

inline void canonicalize(ygg::Data<CallStack>&) noexcept
{
    // Trivially canonical
}

template<tyr::TaskKind Kind>
void canonicalize(ygg::Data<ExecutionState<Kind>>&) noexcept
{
    // Trivially canonical
}

}  // namespace runir::kr::ps::ext

#endif
