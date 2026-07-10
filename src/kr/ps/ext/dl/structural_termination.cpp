#include "runir/kr/ps/ext/dl/structural_termination.hpp"

#include "structural_termination/detail.hpp"

namespace runir::kr::ps::ext::dl
{

ModuleStructuralTerminationResult structural_termination(ModuleView module_)
{
    const auto analysis = detail::analyze_module(module_);
    auto edges = detail::build_policy_edges(analysis);
    const auto [has_cycle, component_of] = detail::sieve_policy_graph(edges, analysis);
    return detail::make_result(analysis, edges, has_cycle, component_of);
}

}  // namespace runir::kr::ps::ext::dl
