#ifndef RUNIR_KR_PS_EXT_DL_INCOMPLETE_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_EXT_DL_INCOMPLETE_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/dl/incomplete_structural_termination_data.hpp"

namespace runir::kr::ps::ext::dl
{

/// Applies the sound but incomplete syntactic proof inside the recurrent
/// components of a module. An unknown result is not a proof of non-termination.
ModuleIncompleteStructuralTerminationResult incomplete_structural_termination(ModuleView module_, bool global_mode = false);

/// Applies the incomplete proof to every module and additionally rejects
/// unresolved or recursive module calls.
ModuleProgramIncompleteStructuralTerminationResult incomplete_structural_termination(ModuleProgramView program, bool global_mode = false);

}  // namespace runir::kr::ps::ext::dl

#endif
