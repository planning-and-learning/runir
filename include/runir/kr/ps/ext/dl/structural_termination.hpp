#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/ext/dl/structural_termination_data.hpp"
#include "runir/kr/ps/ext/module_program_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

namespace runir::kr::ps::ext::dl
{

/// Decides structural termination of the extended sketch module with the
/// Sieve algorithm on the extended policy graph over (feature valuation,
/// memory state) pairs (bonet-et-al-icaps2024.pdf, Theorem 10). Load rules
/// use the paper's Phi(r) semantics: loading register r only unconstrains
/// features that mention r. Do and call rules are treated conservatively
/// because they may change the planning state or transfer control to another
/// module. On failure, the result carries the surviving non-trivial strongly
/// connected components as a counterexample graph.
ModuleStructuralTerminationResult structural_termination(ModuleView module_);

/// Conservative structural termination check for a module program. Each module
/// must structurally terminate, and the inter-module call graph must be
/// acyclic. Recursive module calls are reported non-terminating because the
/// ICAPS 2024 paper leaves modular acyclicity/termination as future work.
ModuleProgramStructuralTerminationResult structural_termination(ModuleProgramView program);

}  // namespace runir::kr::ps::ext::dl

#endif
