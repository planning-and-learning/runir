#ifndef RUNIR_KR_PS_EXT_DL_CEG_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_EXT_DL_CEG_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/ext/dl/structural_termination_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"

namespace runir::kr::ps::ext::dl
{

/// Decides structural termination of the extended sketch module; equivalent
/// to structural_termination() but decomposes by the strongly connected
/// components of the memory graph and projects each component onto the
/// features its rules mention, which is exponentially cheaper whenever the
/// module's features partition across memory components.
ModuleStructuralTerminationResult ceg_structural_termination(ModuleView module_);

}  // namespace runir::kr::ps::ext::dl

#include "runir/kr/ps/ext/dl/ceg_structural_termination_impl.hpp"

#endif
