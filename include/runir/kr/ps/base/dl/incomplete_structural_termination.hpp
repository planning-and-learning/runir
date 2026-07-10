#ifndef RUNIR_KR_PS_BASE_DL_INCOMPLETE_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_BASE_DL_INCOMPLETE_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/base/dl/incomplete_structural_termination_data.hpp"
#include "runir/kr/ps/base/repository.hpp"

namespace runir::kr::ps::base::dl
{

/// Decides structural termination of the sketch with the incomplete
/// syntactic rule-elimination procedure of incomplete_sieve.pdf Section 5.1
/// (cases R1-R3 with feature marking). A "terminating" verdict is sound
/// (Theorem 4); "unknown" is not a proof of non-termination -- use
/// structural_termination() for the complete test. On an unknown verdict,
/// the result lists the surviving rules with their blocking reasons.
IncompleteStructuralTerminationResult incomplete_structural_termination(SketchView sketch);

}  // namespace runir::kr::ps::base::dl

#endif
