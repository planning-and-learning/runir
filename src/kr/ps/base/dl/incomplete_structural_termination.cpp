#include "runir/kr/ps/base/dl/incomplete_structural_termination.hpp"

#include "runir/kr/ps/base/sketch_view.hpp"
#include "structural_termination/detail.hpp"

namespace runir::kr::ps::base::dl
{

IncompleteStructuralTerminationResult incomplete_structural_termination(SketchView sketch, bool use_memory_scc_scope)
{
    const auto analysis = detail::analyze_sketch(sketch);
    const auto policy_result = runir::kr::ps::detail::incomplete_structural_termination(analysis.policy, use_memory_scc_scope);
    return detail::make_incomplete_result(sketch, analysis, policy_result);
}

}  // namespace runir::kr::ps::base::dl
