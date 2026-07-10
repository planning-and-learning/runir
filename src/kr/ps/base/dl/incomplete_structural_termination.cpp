#include "runir/kr/ps/base/dl/incomplete_structural_termination.hpp"

#include "structural_termination/detail.hpp"

namespace runir::kr::ps::base::dl
{

IncompleteStructuralTerminationResult incomplete_structural_termination(SketchView sketch)
{
    const auto analysis = detail::analyze_sketch(sketch);
    const auto sieve_result = detail::run_incomplete_sieve(analysis);
    return detail::make_incomplete_result(sketch, analysis, sieve_result);
}

}  // namespace runir::kr::ps::base::dl
