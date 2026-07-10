#include "runir/kr/ps/base/dl/structural_termination.hpp"

#include "structural_termination/detail.hpp"

namespace runir::kr::ps::base::dl
{

StructuralTerminationResult structural_termination(SketchView sketch)
{
    const auto analysis = detail::analyze_sketch(sketch);
    auto edges = detail::build_policy_edges(analysis);
    const auto [has_cycle, component_of] = detail::sieve_policy_graph(edges, analysis);
    return detail::make_result(analysis, edges, has_cycle, component_of);
}

}  // namespace runir::kr::ps::base::dl
