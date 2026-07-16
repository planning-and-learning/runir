#include "runir/kr/ps/ext/dl/structural_termination.hpp"

#include "runir/kr/ps/ext/module_view.hpp"
#include "structural_termination/detail.hpp"

namespace runir::kr::ps::ext::dl
{

ModuleStructuralTerminationResult structural_termination(ModuleView module_, std::size_t max_features, bool use_incomplete_preprocessing)
{
    const auto analysis = detail::analyze_module(module_);
    if (!use_incomplete_preprocessing)
        return detail::make_result(module_, analysis, runir::kr::ps::detail::sieve_policy(analysis.policy, max_features, false));

    const auto incomplete_result = runir::kr::ps::detail::incomplete_structural_termination(analysis.policy);
    auto result = detail::make_result(module_, analysis, runir::kr::ps::detail::sieve_policy(analysis.policy, max_features, incomplete_result));
    result.incomplete_result = detail::make_incomplete_result(module_, analysis, incomplete_result);
    return result;
}

}  // namespace runir::kr::ps::ext::dl
