#include "detail.hpp"

#include <limits>

namespace runir::kr::ps::base::dl::detail
{

StructuralTerminationResult
make_result(const SketchAnalysis& analysis, const std::vector<PolicyEdge>& edges, bool has_cycle, const std::vector<std::size_t>& component_of)
{
    auto result = StructuralTerminationResult {};
    result.booleans = analysis.features.booleans;
    result.numericals = analysis.features.numericals;
    if (!has_cycle)
    {
        result.status = StructuralTerminationStatus::TERMINATING;
        return result;
    }

    result.status = StructuralTerminationStatus::NON_TERMINATING;
    const auto num_booleans = analysis.features.booleans.size();
    const auto num_numericals = analysis.features.numericals.size();
    const auto num_valuations = std::size_t { 1 } << (num_booleans + num_numericals);
    auto counterexample_builder = PolicyGraphBuilder {};
    auto vertex_remap = std::vector<std::size_t>(num_valuations, std::numeric_limits<std::size_t>::max());
    const auto map_vertex = [&](std::size_t vertex)
    {
        if (vertex_remap[vertex] == std::numeric_limits<std::size_t>::max())
            vertex_remap[vertex] = counterexample_builder.add_vertex(
                PolicyGraphVertexLabel(vertex_booleans(vertex, num_booleans), vertex_numericals(vertex, num_booleans, num_numericals)));
        return static_cast<graphs::VertexIndex>(vertex_remap[vertex]);
    };
    for (const auto& edge : edges)
    {
        if (!edge.alive || component_of[edge.source] != component_of[edge.target])
            continue;
        counterexample_builder.add_directed_edge(
            map_vertex(edge.source),
            map_vertex(edge.target),
            PolicyGraphEdgeLabel(analysis.rules[edge.rule_position], analysis.profiles[edge.rule_position].numerical_changes));
    }
    result.counterexample = std::make_shared<PolicyGraph>(std::move(counterexample_builder));
    return result;
}

}  // namespace runir::kr::ps::base::dl::detail
