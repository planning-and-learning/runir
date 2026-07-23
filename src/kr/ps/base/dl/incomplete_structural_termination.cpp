#include "runir/kr/ps/base/dl/incomplete_structural_termination.hpp"

#include "runir/kr/ps/base/sketch_view.hpp"
#include "structural_termination/detail.hpp"

namespace runir::kr::ps::base::dl
{

IncompleteStructuralTerminationResult incomplete_structural_termination(SketchView sketch, bool global_mode)
{
    const auto analysis = detail::analyze_sketch(sketch);
    const auto policy_result = runir::kr::ps::detail::incomplete_structural_termination(analysis.policy, global_mode);
    return detail::make_incomplete_result(sketch, analysis, policy_result);
}

}  // namespace runir::kr::ps::base::dl
