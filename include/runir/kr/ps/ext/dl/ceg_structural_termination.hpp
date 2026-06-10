#ifndef RUNIR_KR_PS_EXT_DL_CEG_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_EXT_DL_CEG_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/ext/dl/structural_termination.hpp"

#include <utility>

namespace runir::kr::ps::ext::dl
{

/**
 * Component-exploiting structural termination of an extended sketch module.
 *
 * Semantically equivalent to structural_termination() (the same sieve, the
 * same verdicts), but exploits the memory structure for efficiency:
 *
 *  1. Any cycle of the extended policy graph projects to a closed walk of
 *     the memory graph (vertices = memory states, edges = rules), which
 *     stays inside one strongly connected component of the memory graph.
 *     Rules whose source and target memory states lie in different memory
 *     components can never participate in a cycle and are discarded without
 *     ever enumerating valuations.
 *  2. Within one memory component, only the features mentioned by the
 *     component's rules matter: an unmentioned feature is unconstrained on
 *     every edge of the component, so it neither enables nor blocks an edge
 *     removal. The sieve therefore runs on the valuations of the mentioned
 *     features only.
 *
 * Asymptotic saving: instead of one policy graph with 2^|F| * |M| vertices,
 * the analysis runs per memory component c with 2^|F_c| * |M_c| vertices,
 * i.e. sum_c 2^|F_c| * |M_c| total -- exponentially smaller whenever the
 * module's features partition across memory components.
 *
 * Counterexamples agree with structural_termination() up to the canonical
 * projection: features that are unmentioned within a surviving component are
 * reported with value 0 instead of being duplicated across all their
 * (unconstrained) values.
 */
inline ModuleStructuralTerminationResult ceg_structural_termination(ModuleView module_)
{
    auto analysis = detail::analyze_module(module_);
    const auto& memory_states = analysis.memory_states;
    const auto& rules = analysis.rules;
    const auto& features = analysis.features;
    const auto& profiles = analysis.profiles;

    auto result = ModuleStructuralTerminationResult {};
    result.concepts = features.concepts;
    result.booleans = features.booleans;
    result.numericals = features.numericals;

    const auto num_memory_states = memory_states.size();
    if (num_memory_states == 0)
        return result;

    const auto global_num_concepts = features.concepts.size();
    const auto global_num_booleans = features.booleans.size();
    const auto global_num_numerical_like = global_num_concepts + features.numericals.size();
    if (global_num_booleans > 32 || global_num_numerical_like > 32)
        throw std::invalid_argument("ceg_structural_termination: more than 32 features of one kind are not supported.");

    // Strongly connected components of the memory graph.
    auto memory_builder = graphs::StaticGraphBuilder<> {};
    for (std::size_t position = 0; position < num_memory_states; ++position)
        memory_builder.add_vertex();
    for (const auto& profile : profiles)
        memory_builder.add_directed_edge(static_cast<graphs::VertexIndex>(profile.source_memory_position),
                                         static_cast<graphs::VertexIndex>(profile.target_memory_position));
    const auto memory_graph = graphs::StaticGraph<> { std::move(memory_builder) };
    const auto [num_memory_components, memory_component_of] = graphs::algorithms::strong_components(memory_graph);

    auto counterexample_builder = ModulePolicyGraphBuilder {};
    auto has_any_cycle = false;

    for (std::size_t component = 0; component < static_cast<std::size_t>(num_memory_components); ++component)
    {
        // Rules entirely inside this memory component.
        auto component_rules = std::vector<std::size_t> {};
        for (std::size_t rule_position = 0; rule_position < profiles.size(); ++rule_position)
        {
            const auto& profile = profiles[rule_position];
            if (memory_component_of[profile.source_memory_position] == component
                && memory_component_of[profile.target_memory_position] == component)
                component_rules.push_back(rule_position);
        }
        if (component_rules.empty())
            continue;

        // Memory states of this component, with local positions.
        auto component_memory = std::vector<std::size_t> {};
        auto local_memory_position = std::vector<std::size_t>(num_memory_states, 0);
        for (std::size_t position = 0; position < num_memory_states; ++position)
        {
            if (memory_component_of[position] == component)
            {
                local_memory_position[position] = component_memory.size();
                component_memory.push_back(position);
            }
        }
        const auto num_local_memory = component_memory.size();

        // Features mentioned by the component's rules (global positions).
        auto used_booleans = std::uint32_t { 0 };
        auto used_numerical_like = std::uint64_t { 0 };
        for (auto rule_position : component_rules)
        {
            const auto& profile = profiles[rule_position];
            used_booleans |= profile.boolean_positive_conditions | profile.boolean_negative_conditions | profile.boolean_positive_effects
                             | profile.boolean_negative_effects | profile.boolean_unchanged_effects;
            used_numerical_like |= profile.numerical_greater_conditions | profile.numerical_zero_conditions;
            for (std::size_t position = 0; position < global_num_numerical_like; ++position)
                if (static_cast<NumericalChange>((profile.numerical_changes >> (2 * position)) & std::uint64_t { 3 }) != NumericalChange::UNCONSTRAINED)
                    used_numerical_like |= std::uint64_t { 1 } << position;
        }

        auto local_boolean_positions = std::vector<std::size_t> {};  // local -> global
        for (std::size_t position = 0; position < global_num_booleans; ++position)
            if (used_booleans & (std::uint32_t { 1 } << position))
                local_boolean_positions.push_back(position);
        auto local_numerical_positions = std::vector<std::size_t> {};  // local -> global (numerical-like)
        for (std::size_t position = 0; position < global_num_numerical_like; ++position)
            if (used_numerical_like & (std::uint64_t { 1 } << position))
                local_numerical_positions.push_back(position);

        const auto num_local_booleans = local_boolean_positions.size();
        const auto num_local_numerical_like = local_numerical_positions.size();
        if (num_local_booleans + num_local_numerical_like > 14)
            throw std::invalid_argument("ceg_structural_termination: too many features in one memory component.");

        const auto remap_boolean_mask = [&](std::uint32_t global_mask)
        {
            auto local_mask = std::uint32_t { 0 };
            for (std::size_t local = 0; local < num_local_booleans; ++local)
                if (global_mask & (std::uint32_t { 1 } << local_boolean_positions[local]))
                    local_mask |= std::uint32_t { 1 } << local;
            return local_mask;
        };
        const auto remap_numerical_mask = [&](std::uint64_t global_mask)
        {
            auto local_mask = std::uint64_t { 0 };
            for (std::size_t local = 0; local < num_local_numerical_like; ++local)
                if (global_mask & (std::uint64_t { 1 } << local_numerical_positions[local]))
                    local_mask |= std::uint64_t { 1 } << local;
            return local_mask;
        };
        const auto remap_numerical_changes = [&](std::uint64_t global_changes)
        {
            auto local_changes = std::uint64_t { 0 };
            for (std::size_t local = 0; local < num_local_numerical_like; ++local)
                local_changes |= ((global_changes >> (2 * local_numerical_positions[local])) & std::uint64_t { 3 }) << (2 * local);
            return local_changes;
        };

        // Local profiles and policy edges.
        const auto local_boolean_mask = (num_local_booleans ? (std::uint32_t { 1 } << num_local_booleans) : 1u) - 1;
        const auto num_local_valuations = std::uint64_t { 1 } << (num_local_booleans + num_local_numerical_like);

        auto edges = std::vector<detail::ModulePolicyEdge> {};
        for (auto rule_position : component_rules)
        {
            const auto& profile = profiles[rule_position];
            const auto boolean_positive_conditions = remap_boolean_mask(profile.boolean_positive_conditions);
            const auto boolean_negative_conditions = remap_boolean_mask(profile.boolean_negative_conditions);
            const auto numerical_greater_conditions = remap_numerical_mask(profile.numerical_greater_conditions);
            const auto numerical_zero_conditions = remap_numerical_mask(profile.numerical_zero_conditions);
            const auto boolean_positive_effects = remap_boolean_mask(profile.boolean_positive_effects);
            const auto boolean_negative_effects = remap_boolean_mask(profile.boolean_negative_effects);
            const auto boolean_unchanged_effects = remap_boolean_mask(profile.boolean_unchanged_effects);
            const auto numerical_changes = remap_numerical_changes(profile.numerical_changes);
            const auto source_memory = local_memory_position[profile.source_memory_position];
            const auto target_memory = local_memory_position[profile.target_memory_position];

            for (std::uint64_t source = 0; source < num_local_valuations; ++source)
            {
                const auto source_booleans = static_cast<std::uint32_t>(source) & local_boolean_mask;
                const auto source_numericals = source >> num_local_booleans;

                if ((source_booleans & boolean_positive_conditions) != boolean_positive_conditions)
                    continue;
                if ((~source_booleans & boolean_negative_conditions) != boolean_negative_conditions)
                    continue;
                if ((source_numericals & numerical_greater_conditions) != numerical_greater_conditions)
                    continue;
                if ((~source_numericals & numerical_zero_conditions) != numerical_zero_conditions)
                    continue;

                auto forced_booleans = boolean_positive_effects | (boolean_unchanged_effects & source_booleans);
                const auto fixed_boolean_mask = boolean_positive_effects | boolean_negative_effects | boolean_unchanged_effects;

                auto forced_numericals = std::uint64_t { 0 };
                auto fixed_numerical_mask = std::uint64_t { 0 };
                auto decreases_unsatisfiable = false;
                for (std::size_t position = 0; position < num_local_numerical_like; ++position)
                {
                    const auto bit = std::uint64_t { 1 } << position;
                    switch (static_cast<NumericalChange>((numerical_changes >> (2 * position)) & std::uint64_t { 3 }))
                    {
                        case NumericalChange::INCREASES:
                        {
                            forced_numericals |= bit;
                            fixed_numerical_mask |= bit;
                            break;
                        }
                        case NumericalChange::DECREASES:
                        {
                            if (!(source_numericals & bit))
                                decreases_unsatisfiable = true;
                            break;
                        }
                        case NumericalChange::UNCHANGED:
                        {
                            forced_numericals |= source_numericals & bit;
                            fixed_numerical_mask |= bit;
                            break;
                        }
                        case NumericalChange::UNCONSTRAINED: break;
                    }
                }
                if (decreases_unsatisfiable)
                    continue;

                const auto free_boolean_mask = static_cast<std::uint32_t>(~fixed_boolean_mask) & local_boolean_mask;
                const auto numerical_like_mask = (num_local_numerical_like ? (std::uint64_t { 1 } << num_local_numerical_like) : 1u) - 1;
                const auto free_numerical_mask = ~fixed_numerical_mask & numerical_like_mask;

                auto boolean_subset = std::uint32_t { 0 };
                while (true)
                {
                    auto numerical_subset = std::uint64_t { 0 };
                    while (true)
                    {
                        const auto target_booleans = forced_booleans | boolean_subset;
                        const auto target_numericals = forced_numericals | numerical_subset;
                        const auto target_valuation = static_cast<std::uint64_t>(target_booleans) | (target_numericals << num_local_booleans);
                        edges.push_back(detail::ModulePolicyEdge { static_cast<std::uint32_t>(source * num_local_memory + source_memory),
                                                                   static_cast<std::uint32_t>(target_valuation * num_local_memory + target_memory),
                                                                   static_cast<std::uint32_t>(rule_position),
                                                                   numerical_changes });

                        if (numerical_subset == free_numerical_mask)
                            break;
                        numerical_subset = (numerical_subset - free_numerical_mask) & free_numerical_mask;
                    }
                    if (boolean_subset == free_boolean_mask)
                        break;
                    boolean_subset = (boolean_subset - free_boolean_mask) & free_boolean_mask;
                }
            }
        }

        const auto num_local_vertices = static_cast<std::size_t>(num_local_valuations) * num_local_memory;
        const auto [has_cycle, component_of] =
            detail::sieve(edges,
                          num_local_vertices,
                          num_local_numerical_like,
                          [&](std::uint32_t vertex) { return static_cast<std::uint32_t>(vertex / num_local_memory) & local_boolean_mask; });
        if (!has_cycle)
            continue;

        has_any_cycle = true;

        // Append the surviving cycles to the counterexample, mapping local
        // valuations back to the global feature positions (unmentioned
        // features are 0).
        auto vertex_remap = std::vector<std::uint32_t>(num_local_vertices, std::numeric_limits<std::uint32_t>::max());
        const auto map_vertex = [&](std::uint32_t vertex)
        {
            if (vertex_remap[vertex] == std::numeric_limits<std::uint32_t>::max())
            {
                const auto valuation = static_cast<std::uint64_t>(vertex / num_local_memory);
                const auto memory = component_memory[static_cast<std::size_t>(vertex % num_local_memory)];
                const auto local_booleans = static_cast<std::uint32_t>(valuation) & local_boolean_mask;
                const auto local_numericals = valuation >> num_local_booleans;

                auto global_booleans = std::uint32_t { 0 };
                for (std::size_t local = 0; local < num_local_booleans; ++local)
                    if (local_booleans & (std::uint32_t { 1 } << local))
                        global_booleans |= std::uint32_t { 1 } << local_boolean_positions[local];
                auto global_numerical_like = std::uint64_t { 0 };
                for (std::size_t local = 0; local < num_local_numerical_like; ++local)
                    if (local_numericals & (std::uint64_t { 1 } << local))
                        global_numerical_like |= std::uint64_t { 1 } << local_numerical_positions[local];

                const auto concepts =
                    static_cast<std::uint32_t>(global_numerical_like & ((global_num_concepts ? (std::uint64_t { 1 } << global_num_concepts) : 1u) - 1));
                const auto numericals = static_cast<std::uint32_t>(global_numerical_like >> global_num_concepts);
                vertex_remap[vertex] =
                    counterexample_builder.add_vertex(ModulePolicyGraphVertexLabel(concepts, global_booleans, numericals, memory_states[memory]));
            }
            return vertex_remap[vertex];
        };
        for (const auto& edge : edges)
        {
            if (!edge.alive || component_of[edge.source] != component_of[edge.target])
                continue;
            counterexample_builder.add_directed_edge(
                map_vertex(edge.source),
                map_vertex(edge.target),
                ModulePolicyGraphEdgeLabel(rules[edge.rule_position], profiles[edge.rule_position].numerical_changes));
        }
    }

    if (!has_any_cycle)
    {
        result.status = StructuralTerminationStatus::TERMINATING;
        return result;
    }

    result.status = StructuralTerminationStatus::NON_TERMINATING;
    result.counterexample = std::make_shared<ModulePolicyGraph>(std::move(counterexample_builder));
    return result;
}

}  // namespace runir::kr::ps::ext::dl

#endif
