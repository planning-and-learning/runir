#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/ext/dl/structural_termination_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"

namespace runir::kr::ps::ext::dl
{

/// Decides structural termination of the extended sketch module with the
/// Sieve algorithm on the extended policy graph over (feature valuation,
/// memory state) pairs (bonet-et-al-icaps2024.pdf, Theorem 10). Load, do,
/// and call rules are treated conservatively (all features unconstrained),
/// so "terminating" verdicts are sound. On failure, the result carries the
/// surviving non-trivial strongly connected components as a counterexample
/// graph.
ModuleStructuralTerminationResult structural_termination(ModuleView module_);

}  // namespace runir::kr::ps::ext::dl

#include "runir/kr/ps/ext/dl/structural_termination_impl.hpp"

#endif
