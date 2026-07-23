#include "runir/kr/ps/base/dl/structural_termination.hpp"

#include "runir/kr/ps/base/sketch_view.hpp"
#include "structural_termination/detail.hpp"

namespace runir::kr::ps::base::dl
{

StructuralTerminationResult structural_termination(SketchView sketch, std::size_t max_features, bool use_incomplete_preprocessing, bool use_memory_scc_scope)
{
    const auto analysis = detail::analyze_sketch(sketch);
    if (!use_incomplete_preprocessing)
        return detail::make_result(sketch, analysis, runir::kr::ps::detail::sieve_policy(analysis.policy, max_features, false));

    const auto incomplete_result = runir::kr::ps::detail::incomplete_structural_termination(analysis.policy, use_memory_scc_scope);
    auto result = detail::make_result(sketch, analysis, runir::kr::ps::detail::sieve_policy(analysis.policy, max_features, incomplete_result));
    result.incomplete_result = detail::make_incomplete_result(sketch, analysis, incomplete_result);
    return result;
}

}  // namespace runir::kr::ps::base::dl
