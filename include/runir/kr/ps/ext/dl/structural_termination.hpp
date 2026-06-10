#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/ext/repository.hpp"

namespace runir::kr::ps::ext::dl
{

/**
 * Structural termination of a module program.
 *
 * Unlike the base family, termination of a module depends on the memory
 * structure: a sieve over feature effects must argue about the cycles of the
 * memory transition graph, since rule effects only constrain features along
 * the transitions they label. Conservatively reports false (not proven
 * terminating) until the cycle-based analysis is implemented.
 */
inline bool is_structurally_terminating(ModuleView) { return false; }

}  // namespace runir::kr::ps::ext::dl

#endif
