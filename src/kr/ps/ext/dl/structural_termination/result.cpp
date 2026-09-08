#include "detail.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <limits>

namespace runir::kr::ps::ext::dl::detail
{

ModuleStructuralTerminationResult make_result(ModuleView module_, const ModuleAnalysis& analysis, const runir::kr::ps::detail::PolicySieveResult& sieve_result)
{
    auto result = ModuleStructuralTerminationResult {};
    result.scc_results =
        runir::kr::ps::detail::materialize_scc_results<runir::kr::ExtFamilyTag, Repository>(sieve_result.scc_feature_positions,
                                                                                            module_.get_features<runir::kr::ps::dl::BooleanFeature>(),
                                                                                            module_.get_features<runir::kr::ps::dl::NumericalFeature>());
    if (sieve_result.components.empty())
    {
        result.status = StructuralTerminationStatus::TERMINATING;
        return result;
    }

    result.status = StructuralTerminationStatus::NON_TERMINATING;
    auto counterexample_builder = ModulePolicyGraphBuilder {};
    for (const auto& component : sieve_result.components)
    {
        const auto& projected = component.projected;
        auto vertex_remap = std::vector<std::size_t>(projected.policy.num_vertices(), std::numeric_limits<std::size_t>::max());
        const auto map_vertex = [&](std::size_t vertex)
        {
            if (vertex_remap[vertex] == std::numeric_limits<std::size_t>::max())
            {
                auto [booleans, numericals] = runir::kr::ps::detail::unproject_vertex(vertex, projected, analysis.policy);
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
                map_vertex(edge.source), map_vertex(edge.target), analysis.rules[rule_position]);
        }
    }
    result.counterexample = std::make_shared<ModulePolicyGraph>(std::move(counterexample_builder));
    return result;
}

}  // namespace runir::kr::ps::ext::dl::detail
