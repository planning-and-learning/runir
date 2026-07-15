#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/dl/structural_termination_data.hpp"

namespace runir::kr::ps::ext::dl
{

/// Decides structural termination of the extended sketch module with the
/// incomplete syntactic elimination followed by complete Sieve on projected
/// residual memory components. The policy graphs have (feature valuation,
/// memory state) vertices (bonet-et-al-icaps2024.pdf, Theorem 10). Load rules
/// use the paper's Phi(r) semantics: loading register r only unconstrains
/// features that mention r. Do and call rules are treated conservatively
/// because they may change the planning state or transfer control to another
/// module. On failure, the result carries a canonical counterexample graph
/// lifted onto the module's full feature axes. The incomplete preprocessing
/// can be disabled.
ModuleStructuralTerminationResult structural_termination(ModuleView module_,
                                                         std::size_t max_features = runir::kr::ps::dl::default_max_features,
                                                         bool use_incomplete_preprocessing = runir::kr::ps::dl::default_use_incomplete_preprocessing);

/// Conservative structural termination check for a module program. Each module
/// must structurally terminate, and the inter-module call graph must be
/// acyclic. Recursive module calls are reported non-terminating because the
/// ICAPS 2024 paper leaves modular acyclicity/termination as future work.
ModuleProgramStructuralTerminationResult structural_termination(ModuleProgramView program,
                                                                std::size_t max_features = runir::kr::ps::dl::default_max_features,
                                                                bool use_incomplete_preprocessing = runir::kr::ps::dl::default_use_incomplete_preprocessing);

}  // namespace runir::kr::ps::ext::dl

#endif
