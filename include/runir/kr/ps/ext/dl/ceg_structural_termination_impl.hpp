#ifndef RUNIR_KR_PS_EXT_DL_CEG_STRUCTURAL_TERMINATION_IMPL_HPP_
#define RUNIR_KR_PS_EXT_DL_CEG_STRUCTURAL_TERMINATION_IMPL_HPP_

#include "runir/kr/ps/ext/dl/ceg_structural_termination.hpp"
#include "runir/kr/ps/ext/dl/structural_termination.hpp"

#include <utility>

namespace runir::kr::ps::ext::dl
{

/*
 * Implementation notes.
 *
 * Semantically equivalent to structural_termination() (the same sieve, the
 * same verdicts), but exploits the memory structure:
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
    result.booleans = features.booleans;
    result.numericals = features.numericals;

    const auto num_memory_states = memory_states.size();
    if (num_memory_states == 0)
        return result;

    const auto global_num_booleans = features.booleans.size();
    const auto global_num_numericals = features.numericals.size();

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
        auto used_booleans = boost::dynamic_bitset<>(global_num_booleans);
        auto used_numericals = boost::dynamic_bitset<>(global_num_numericals);
        for (auto rule_position : component_rules)
        {
            const auto& profile = profiles[rule_position];
            used_booleans |= profile.boolean_positive_conditions | profile.boolean_negative_conditions | profile.boolean_positive_effects
                             | profile.boolean_negative_effects | profile.boolean_unchanged_effects;
            used_numericals |= profile.numerical_greater_conditions | profile.numerical_zero_conditions;
            for (std::size_t position = 0; position < global_num_numericals; ++position)
                if (profile.numerical_changes[position] != NumericalChange::UNCONSTRAINED)
                    used_numericals.set(position);
        }

        auto local_boolean_positions = std::vector<std::size_t> {};  // local -> global
        for (std::size_t position = 0; position < global_num_booleans; ++position)
            if (used_booleans.test(position))
                local_boolean_positions.push_back(position);
        auto local_numerical_positions = std::vector<std::size_t> {};  // local -> global (numerical)
        for (std::size_t position = 0; position < global_num_numericals; ++position)
            if (used_numericals.test(position))
                local_numerical_positions.push_back(position);

        const auto num_local_booleans = local_boolean_positions.size();
        const auto num_local_numericals = local_numerical_positions.size();
        if (num_local_booleans + num_local_numericals > 14)
            throw std::invalid_argument("ceg_structural_termination: too many features in one memory component.");

        const auto remap_boolean_values = [&](const boost::dynamic_bitset<>& global_values)
        {
            auto local_values = boost::dynamic_bitset<>(num_local_booleans);
            for (std::size_t local = 0; local < num_local_booleans; ++local)
                local_values.set(local, global_values.test(local_boolean_positions[local]));
            return local_values;
        };
        const auto remap_numerical_values = [&](const boost::dynamic_bitset<>& global_values)
        {
            auto local_values = boost::dynamic_bitset<>(num_local_numericals);
            for (std::size_t local = 0; local < num_local_numericals; ++local)
                local_values.set(local, global_values.test(local_numerical_positions[local]));
            return local_values;
        };

        // Local profiles over the component's features and memory states.
        auto local_profiles = std::vector<detail::ModuleRuleProfile> {};
        auto local_rule_positions = std::vector<std::size_t> {};  // local profile -> global rule position
        for (auto rule_position : component_rules)
        {
            const auto& profile = profiles[rule_position];
            auto local_profile = detail::ModuleRuleProfile(num_local_booleans, num_local_numericals);
            local_profile.source_memory_position = local_memory_position[profile.source_memory_position];
            local_profile.target_memory_position = local_memory_position[profile.target_memory_position];
            local_profile.boolean_positive_conditions = remap_boolean_values(profile.boolean_positive_conditions);
            local_profile.boolean_negative_conditions = remap_boolean_values(profile.boolean_negative_conditions);
            local_profile.numerical_greater_conditions = remap_numerical_values(profile.numerical_greater_conditions);
            local_profile.numerical_zero_conditions = remap_numerical_values(profile.numerical_zero_conditions);
            local_profile.boolean_positive_effects = remap_boolean_values(profile.boolean_positive_effects);
            local_profile.boolean_negative_effects = remap_boolean_values(profile.boolean_negative_effects);
            local_profile.boolean_unchanged_effects = remap_boolean_values(profile.boolean_unchanged_effects);
            for (std::size_t local = 0; local < num_local_numericals; ++local)
                local_profile.numerical_changes[local] = profile.numerical_changes[local_numerical_positions[local]];
            local_rule_positions.push_back(rule_position);
            local_profiles.push_back(std::move(local_profile));
        }

        auto edges = std::vector<detail::ModulePolicyEdge> {};
        for (std::size_t local_rule = 0; local_rule < local_profiles.size(); ++local_rule)
            detail::enumerate_rule_edges(local_profiles[local_rule],
                                         num_local_memory,
                                         num_local_booleans,
                                         num_local_numericals,
                                         [&](std::size_t source, std::size_t target)
                                         { edges.push_back(detail::ModulePolicyEdge { source, target, local_rule }); });

        const auto num_local_vertices = (std::size_t { 1 } << (num_local_booleans + num_local_numericals)) * num_local_memory;
        const auto [has_cycle, component_of] =
            detail::sieve(edges,
                          num_local_vertices,
                          num_local_booleans,
                          num_local_numericals,
                          [&](std::size_t vertex) { return detail::vertex_booleans(vertex, num_local_memory, num_local_booleans); },
                          [&](const detail::ModulePolicyEdge& edge) -> const std::vector<NumericalChange>&
                          { return local_profiles[edge.rule_position].numerical_changes; });
        if (!has_cycle)
            continue;

        has_any_cycle = true;

        // Append the surviving cycles to the counterexample, mapping local
        // valuations back to the global feature positions (unmentioned
        // features are 0).
        auto vertex_remap = std::vector<std::size_t>(num_local_vertices, std::numeric_limits<std::size_t>::max());
        const auto map_vertex = [&](std::size_t vertex)
        {
            if (vertex_remap[vertex] == std::numeric_limits<std::size_t>::max())
            {
                const auto local_booleans = detail::vertex_booleans(vertex, num_local_memory, num_local_booleans);
                const auto local_numericals = detail::vertex_numericals(vertex, num_local_memory, num_local_booleans, num_local_numericals);

                auto global_booleans = boost::dynamic_bitset<>(global_num_booleans);
                for (std::size_t local = 0; local < num_local_booleans; ++local)
                    global_booleans.set(local_boolean_positions[local], local_booleans.test(local));
                auto global_numericals = boost::dynamic_bitset<>(features.numericals.size());
                for (std::size_t local = 0; local < num_local_numericals; ++local)
                    global_numericals.set(local_numerical_positions[local], local_numericals.test(local));

                const auto memory = component_memory[static_cast<std::size_t>(vertex % num_local_memory)];
                vertex_remap[vertex] = counterexample_builder.add_vertex(ModulePolicyGraphVertexLabel(std::move(global_booleans),
                                                                                                      std::move(global_numericals),
                                                                                                      memory_states[memory]));
            }
            return static_cast<graphs::VertexIndex>(vertex_remap[vertex]);
        };
        for (const auto& edge : edges)
        {
            if (!edge.alive || component_of[edge.source] != component_of[edge.target])
                continue;
            const auto rule_position = local_rule_positions[edge.rule_position];
            counterexample_builder.add_directed_edge(map_vertex(edge.source),
                                                     map_vertex(edge.target),
                                                     ModulePolicyGraphEdgeLabel(rules[rule_position], profiles[rule_position].numerical_changes));
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
