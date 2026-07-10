#include "runir/kr/ps/ext/dl/ceg_structural_termination.hpp"

#include "structural_termination/detail.hpp"

namespace runir::kr::ps::ext::dl
{

ModuleStructuralTerminationResult ceg_structural_termination(ModuleView module_)
{
    const auto analysis = detail::analyze_module(module_);
    auto result = ModuleStructuralTerminationResult {};
    result.booleans = analysis.features.booleans;
    result.numericals = analysis.features.numericals;

    if (analysis.memory_states.empty())
        return result;

    auto counterexample_builder = ModulePolicyGraphBuilder {};
    auto has_cycle = false;
    for (const auto& component : detail::analyze_ceg_components(analysis))
    {
        auto edges = detail::build_policy_edges(component.profiles,
                                                component.memory_positions.size(),
                                                component.boolean_positions.size(),
                                                component.numerical_positions.size());
        const auto [component_has_cycle, component_of] = detail::sieve_policy_graph(edges,
                                                                                    component.profiles,
                                                                                    component.memory_positions.size(),
                                                                                    component.boolean_positions.size(),
                                                                                    component.numerical_positions.size());
        if (!component_has_cycle)
            continue;

        has_cycle = true;
        detail::append_ceg_counterexample(analysis, component, edges, component_of, counterexample_builder);
    }

    if (has_cycle)
    {
        result.status = StructuralTerminationStatus::NON_TERMINATING;
        result.counterexample = std::make_shared<ModulePolicyGraph>(std::move(counterexample_builder));
    }
    return result;
}

}  // namespace runir::kr::ps::ext::dl
