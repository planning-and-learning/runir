#include "detail.hpp"

#include <limits>

namespace runir::kr::ps::base::dl::detail
{

StructuralTerminationResult
make_result(const SketchAnalysis& analysis, const std::vector<runir::kr::ps::detail::PolicyEdge>& edges, const runir::kr::ps::detail::SieveResult& sieve_result)
{
    auto result = StructuralTerminationResult {};
    result.booleans = analysis.features.booleans;
    result.numericals = analysis.features.numericals;
    if (!sieve_result.has_cycle)
    {
        result.status = StructuralTerminationStatus::TERMINATING;
        return result;
    }

    result.status = StructuralTerminationStatus::NON_TERMINATING;
    auto counterexample_builder = PolicyGraphBuilder {};
    auto vertex_remap = std::vector<std::size_t>(analysis.policy.num_vertices(), std::numeric_limits<std::size_t>::max());
    const auto map_vertex = [&](std::size_t vertex)
    {
        if (vertex_remap[vertex] == std::numeric_limits<std::size_t>::max())
            vertex_remap[vertex] = counterexample_builder.add_vertex(PolicyGraphVertexLabel(runir::kr::ps::detail::vertex_booleans(vertex, analysis.policy),
                                                                                            runir::kr::ps::detail::vertex_numericals(vertex, analysis.policy)));
        return static_cast<graphs::VertexIndex>(vertex_remap[vertex]);
    };
    for (const auto& edge : edges)
    {
        if (!edge.alive || sieve_result.component_of[edge.source] != sieve_result.component_of[edge.target])
            continue;
        counterexample_builder.add_directed_edge(
            map_vertex(edge.source),
            map_vertex(edge.target),
            PolicyGraphEdgeLabel(analysis.rules[edge.rule_position], analysis.policy.rule_profiles[edge.rule_position].numerical_changes));
    }
    result.counterexample = std::make_shared<PolicyGraph>(std::move(counterexample_builder));
    return result;
}

}  // namespace runir::kr::ps::base::dl::detail
