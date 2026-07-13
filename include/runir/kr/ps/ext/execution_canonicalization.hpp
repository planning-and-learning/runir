#ifndef RUNIR_KR_PS_EXT_EXECUTION_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_CANONICALIZATION_HPP_

#include "runir/kr/ps/ext/execution_data.hpp"

namespace runir::kr::ps::ext
{

inline bool is_canonical(const ygg::Data<RegisterValues>&) noexcept { return true; }

inline bool is_canonical(const ygg::Data<CallArguments>&) noexcept { return true; }

inline bool is_canonical(const ygg::Data<CallStack>&) noexcept { return true; }

template<tyr::planning::TaskKind Kind>
bool is_canonical(const ygg::Data<ExecutionState<Kind>>&) noexcept
{
    return true;
}

inline void canonicalize(ygg::Data<RegisterValues>&) noexcept
{
    // Trivially canonical
}

inline void canonicalize(ygg::Data<CallArguments>&) noexcept
{
    // Trivially canonical
}

inline void canonicalize(ygg::Data<CallStack>&) noexcept
{
    // Trivially canonical
}

template<tyr::planning::TaskKind Kind>
void canonicalize(ygg::Data<ExecutionState<Kind>>&) noexcept
{
    // Trivially canonical
}

}  // namespace runir::kr::ps::ext

#endif
