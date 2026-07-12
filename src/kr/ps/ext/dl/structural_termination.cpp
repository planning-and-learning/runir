#include "runir/kr/ps/ext/dl/structural_termination.hpp"

#include "structural_termination/detail.hpp"

#include <stdexcept>

namespace runir::kr::ps::ext::dl
{

ModuleStructuralTerminationResult structural_termination(ModuleView module_)
{
    const auto analysis = detail::analyze_module(module_);
    if (analysis.policy.num_booleans + analysis.policy.num_numericals > 14)
        throw std::invalid_argument("structural_termination: too many features; the policy graph has 2^|features| * |memory| vertices.");

    auto edges = runir::kr::ps::detail::build_policy_edges(analysis.policy);
    const auto sieve_result = runir::kr::ps::detail::sieve_policy_graph(edges, analysis.policy);
    return detail::make_result(analysis, edges, sieve_result);
}

}  // namespace runir::kr::ps::ext::dl
