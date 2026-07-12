#include "runir/kr/ps/base/dl/structural_termination.hpp"

#include "structural_termination/detail.hpp"

namespace runir::kr::ps::base::dl
{

StructuralTerminationResult structural_termination(SketchView sketch, std::size_t max_features, bool use_incomplete_preprocessing)
{
    const auto analysis = detail::analyze_sketch(sketch);
    return detail::make_result(analysis, runir::kr::ps::detail::sieve_policy(analysis.policy, max_features, use_incomplete_preprocessing));
}

}  // namespace runir::kr::ps::base::dl
