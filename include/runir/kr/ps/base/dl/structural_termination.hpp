#ifndef RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/base/dl/structural_termination_data.hpp"
#include "runir/kr/ps/base/repository.hpp"

namespace runir::kr::ps::base::dl
{

/// Decides structural termination by first applying the incomplete syntactic
/// elimination and then the complete Sieve algorithm (sieve.pdf Algorithm 1,
/// extended to Boolean features per incomplete_sieve.pdf Section 5.1) to the
/// residual policy graph. On failure, the result carries a surviving
/// counterexample graph. The incomplete preprocessing can be disabled.
StructuralTerminationResult structural_termination(SketchView sketch,
                                                   std::size_t max_features = runir::kr::ps::dl::default_max_features,
                                                   bool use_incomplete_preprocessing = runir::kr::ps::dl::default_use_incomplete_preprocessing);

}  // namespace runir::kr::ps::base::dl

#endif
