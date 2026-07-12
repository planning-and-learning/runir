#include "detail.hpp"

#include <limits>

namespace runir::kr::ps::ext::dl::detail
{

ModuleStructuralTerminationResult make_result(const ModuleAnalysis& analysis, const runir::kr::ps::detail::ComponentSieveResult& sieve_result)
{
    auto result = ModuleStructuralTerminationResult {};
    result.booleans = analysis.features.booleans;
    result.numericals = analysis.features.numericals;
    if (sieve_result.empty())
    {
        result.status = StructuralTerminationStatus::TERMINATING;
        return result;
    }

    result.status = StructuralTerminationStatus::NON_TERMINATING;
    auto counterexample_builder = ModulePolicyGraphBuilder {};
    for (const auto& component : sieve_result)
    {
        const auto& projected = component.projected;
        auto vertex_remap = std::vector<std::size_t>(projected.policy.num_vertices(), std::numeric_limits<std::size_t>::max());
        const auto map_vertex = [&](std::size_t vertex)
        {
            if (vertex_remap[vertex] == std::numeric_limits<std::size_t>::max())
            {
                auto booleans = boost::dynamic_bitset<>(analysis.policy.num_booleans);
                const auto local_booleans = runir::kr::ps::detail::vertex_booleans(vertex, projected.policy);
                for (std::size_t local = 0; local < projected.boolean_positions.size(); ++local)
                    booleans.set(projected.boolean_positions[local], local_booleans.test(local));

                auto numericals = boost::dynamic_bitset<>(analysis.policy.num_numericals);
                const auto local_numericals = runir::kr::ps::detail::vertex_numericals(vertex, projected.policy);
                for (std::size_t local = 0; local < projected.numerical_positions.size(); ++local)
                    numericals.set(projected.numerical_positions[local], local_numericals.test(local));
                vertex_remap[vertex] = counterexample_builder.add_vertex(
                    ModulePolicyGraphVertexLabel(std::move(booleans),
                                                 std::move(numericals),
                                                 analysis.memory_states[projected.memory_positions[vertex % projected.policy.num_memory_states]]));
            }
            return static_cast<graphs::VertexIndex>(vertex_remap[vertex]);
        };
        for (const auto& edge : component.edges)
        {
            if (!edge.alive || component.sieve.component_of[edge.source] != component.sieve.component_of[edge.target])
                continue;
            const auto rule_position = projected.rule_positions[edge.rule_position];
            counterexample_builder.add_directed_edge(
                map_vertex(edge.source),
                map_vertex(edge.target),
                ModulePolicyGraphEdgeLabel(analysis.rules[rule_position], analysis.policy.rule_profiles[rule_position].numerical_changes));
        }
    }
    result.counterexample = std::make_shared<ModulePolicyGraph>(std::move(counterexample_builder));
    return result;
}

}  // namespace runir::kr::ps::ext::dl::detail
