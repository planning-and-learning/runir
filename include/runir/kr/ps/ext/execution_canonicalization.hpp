#ifndef RUNIR_KR_PS_EXT_EXECUTION_CANONICALIZATION_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_CANONICALIZATION_HPP_

#include "runir/kr/dl/semantics/canonicalization.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
bool is_canonical(const ygg::Data<ModuleState<Kind>>&) noexcept
{
    return true;
}

inline bool is_canonical(const ygg::Data<CallStack>&) noexcept { return true; }

template<tyr::TaskKind Kind>
bool is_canonical(const ygg::Data<ProgramState<Kind>>&) noexcept
{
    return true;
}

template<tyr::TaskKind Kind>
void canonicalize(ygg::Data<ModuleState<Kind>>&) noexcept
{
    // Trivially canonical
}

inline void canonicalize(ygg::Data<CallStack>&) noexcept
{
    // Trivially canonical
}

template<tyr::TaskKind Kind>
void canonicalize(ygg::Data<ProgramState<Kind>>&) noexcept
{
    // Trivially canonical
}

}  // namespace runir::kr::ps::ext

#endif
