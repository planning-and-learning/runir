#include "detail.hpp"

#include <limits>

namespace runir::kr::ps::ext::dl::detail
{

void append_ceg_counterexample(const ModuleAnalysis& analysis,
                               const CegComponentAnalysis& component,
                               const std::vector<ModulePolicyEdge>& edges,
                               const std::vector<std::size_t>& component_of,
                               ModulePolicyGraphBuilder& builder)
{
    const auto num_memory_states = component.memory_positions.size();
    const auto num_booleans = component.boolean_positions.size();
    const auto num_numericals = component.numerical_positions.size();
    const auto num_vertices = (std::size_t { 1 } << (num_booleans + num_numericals)) * num_memory_states;
    auto vertex_remap = std::vector<std::size_t>(num_vertices, std::numeric_limits<std::size_t>::max());

    const auto map_vertex = [&](std::size_t vertex)
    {
        if (vertex_remap[vertex] == std::numeric_limits<std::size_t>::max())
        {
            const auto local_booleans = vertex_booleans(vertex, num_memory_states, num_booleans);
            const auto local_numericals = vertex_numericals(vertex, num_memory_states, num_booleans, num_numericals);

            auto global_booleans = boost::dynamic_bitset<>(analysis.features.booleans.size());
            for (std::size_t local = 0; local < num_booleans; ++local)
                global_booleans.set(component.boolean_positions[local], local_booleans.test(local));
            auto global_numericals = boost::dynamic_bitset<>(analysis.features.numericals.size());
            for (std::size_t local = 0; local < num_numericals; ++local)
                global_numericals.set(component.numerical_positions[local], local_numericals.test(local));

            const auto memory_position = component.memory_positions[vertex % num_memory_states];
            vertex_remap[vertex] = builder.add_vertex(
                ModulePolicyGraphVertexLabel(std::move(global_booleans), std::move(global_numericals), analysis.memory_states[memory_position]));
        }
        return static_cast<graphs::VertexIndex>(vertex_remap[vertex]);
    };

    for (const auto& edge : edges)
    {
        if (!edge.alive || component_of[edge.source] != component_of[edge.target])
            continue;
        const auto rule_position = component.rule_positions[edge.rule_position];
        builder.add_directed_edge(map_vertex(edge.source),
                                  map_vertex(edge.target),
                                  ModulePolicyGraphEdgeLabel(analysis.rules[rule_position], analysis.profiles[rule_position].numerical_changes));
    }
}

}  // namespace runir::kr::ps::ext::dl::detail
