#include "runir/kr/ps/ext/dl/structural_termination.hpp"

#include "structural_termination/detail.hpp"

namespace runir::kr::ps::ext::dl
{

ModuleStructuralTerminationResult structural_termination(ModuleView module_, std::size_t max_features, bool use_incomplete_preprocessing)
{
    const auto analysis = detail::analyze_module(module_);
    return detail::make_result(analysis, runir::kr::ps::detail::sieve_policy(analysis.policy, max_features, use_incomplete_preprocessing));
}

}  // namespace runir::kr::ps::ext::dl
