#include "detail.hpp"

#include <limits>

namespace runir::kr::ps::ext::dl::detail
{

ModuleStructuralTerminationResult
make_result(const ModuleAnalysis& analysis, const std::vector<ModulePolicyEdge>& edges, bool has_cycle, const std::vector<std::size_t>& component_of)
{
    auto result = ModuleStructuralTerminationResult {};
    result.booleans = analysis.features.booleans;
    result.numericals = analysis.features.numericals;
    if (!has_cycle)
    {
        result.status = StructuralTerminationStatus::TERMINATING;
        return result;
    }

    result.status = StructuralTerminationStatus::NON_TERMINATING;
    const auto num_memory_states = analysis.memory_states.size();
    const auto num_booleans = analysis.features.booleans.size();
    const auto num_numericals = analysis.features.numericals.size();
    const auto num_vertices = (std::size_t { 1 } << (num_booleans + num_numericals)) * num_memory_states;
    auto counterexample_builder = ModulePolicyGraphBuilder {};
    auto vertex_remap = std::vector<std::size_t>(num_vertices, std::numeric_limits<std::size_t>::max());
    const auto map_vertex = [&](std::size_t vertex)
    {
        if (vertex_remap[vertex] == std::numeric_limits<std::size_t>::max())
        {
            auto numericals = vertex_numericals(vertex, num_memory_states, num_booleans, num_numericals);
            vertex_remap[vertex] = counterexample_builder.add_vertex(ModulePolicyGraphVertexLabel(vertex_booleans(vertex, num_memory_states, num_booleans),
                                                                                                  std::move(numericals),
                                                                                                  analysis.memory_states[vertex % num_memory_states]));
        }
        return static_cast<graphs::VertexIndex>(vertex_remap[vertex]);
    };
    for (const auto& edge : edges)
    {
        if (!edge.alive || component_of[edge.source] != component_of[edge.target])
            continue;
        counterexample_builder.add_directed_edge(
            map_vertex(edge.source),
            map_vertex(edge.target),
            ModulePolicyGraphEdgeLabel(analysis.rules[edge.rule_position], analysis.profiles[edge.rule_position].numerical_changes));
    }
    result.counterexample = std::make_shared<ModulePolicyGraph>(std::move(counterexample_builder));
    return result;
}

}  // namespace runir::kr::ps::ext::dl::detail
