#ifndef RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/base/dl/structural_termination_data.hpp"
#include "runir/kr/ps/base/repository.hpp"

namespace runir::kr::ps::base::dl
{

/// Decides structural termination of the sketch with the complete Sieve
/// algorithm (sieve.pdf Algorithm 1, extended to Boolean features per
/// incomplete_sieve.pdf Section 5.1) on the policy graph over the sketch's
/// feature valuations. On failure, the result carries the surviving
/// non-trivial strongly connected components as a counterexample graph.
StructuralTerminationResult structural_termination(SketchView sketch);

}  // namespace runir::kr::ps::base::dl

#endif
