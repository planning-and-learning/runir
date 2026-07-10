#include "detail.hpp"
#include "runir/graphs/algorithms.hpp"

#include <stdexcept>

namespace runir::kr::ps::ext::dl::detail
{

std::vector<CegComponentAnalysis> analyze_ceg_components(const ModuleAnalysis& analysis)
{
    const auto num_memory_states = analysis.memory_states.size();
    auto memory_builder = graphs::StaticGraphBuilder<> {};
    for (std::size_t position = 0; position < num_memory_states; ++position)
        memory_builder.add_vertex();
    for (const auto& profile : analysis.profiles)
        memory_builder.add_directed_edge(static_cast<graphs::VertexIndex>(profile.source_memory_position),
                                         static_cast<graphs::VertexIndex>(profile.target_memory_position));
    const auto memory_graph = graphs::StaticGraph<> { std::move(memory_builder) };
    const auto [num_components, component_of] = graphs::algorithms::strong_components(memory_graph);

    auto result = std::vector<CegComponentAnalysis> {};
    for (std::size_t component = 0; component < static_cast<std::size_t>(num_components); ++component)
    {
        auto component_rules = std::vector<std::size_t> {};
        for (std::size_t rule_position = 0; rule_position < analysis.profiles.size(); ++rule_position)
        {
            const auto& profile = analysis.profiles[rule_position];
            if (component_of[profile.source_memory_position] == component && component_of[profile.target_memory_position] == component)
                component_rules.push_back(rule_position);
        }
        if (component_rules.empty())
            continue;

        auto component_analysis = CegComponentAnalysis {};
        auto local_memory_position = std::vector<std::size_t>(num_memory_states, 0);
        for (std::size_t position = 0; position < num_memory_states; ++position)
        {
            if (component_of[position] == component)
            {
                local_memory_position[position] = component_analysis.memory_positions.size();
                component_analysis.memory_positions.push_back(position);
            }
        }

        auto used_booleans = boost::dynamic_bitset<>(analysis.features.booleans.size());
        auto used_numericals = boost::dynamic_bitset<>(analysis.features.numericals.size());
        for (auto rule_position : component_rules)
        {
            const auto& profile = analysis.profiles[rule_position];
            used_booleans |= profile.boolean_positive_conditions | profile.boolean_negative_conditions | profile.boolean_positive_effects
                             | profile.boolean_negative_effects | profile.boolean_unchanged_effects;
            used_numericals |= profile.numerical_greater_conditions | profile.numerical_zero_conditions;
            for (std::size_t position = 0; position < analysis.features.numericals.size(); ++position)
                if (profile.numerical_changes[position] != NumericalChange::UNCONSTRAINED)
                    used_numericals.set(position);
        }

        for (std::size_t position = 0; position < analysis.features.booleans.size(); ++position)
            if (used_booleans.test(position))
                component_analysis.boolean_positions.push_back(position);
        for (std::size_t position = 0; position < analysis.features.numericals.size(); ++position)
            if (used_numericals.test(position))
                component_analysis.numerical_positions.push_back(position);

        const auto num_booleans = component_analysis.boolean_positions.size();
        const auto num_numericals = component_analysis.numerical_positions.size();
        if (num_booleans + num_numericals > 14)
            throw std::invalid_argument("ceg_structural_termination: too many features in one memory component.");

        const auto remap = [](const boost::dynamic_bitset<>& global_values, const std::vector<std::size_t>& positions)
        {
            auto local_values = boost::dynamic_bitset<>(positions.size());
            for (std::size_t local = 0; local < positions.size(); ++local)
                local_values.set(local, global_values.test(positions[local]));
            return local_values;
        };

        for (auto rule_position : component_rules)
        {
            const auto& profile = analysis.profiles[rule_position];
            auto local_profile = ModuleRuleProfile(num_booleans, num_numericals);
            local_profile.source_memory_position = local_memory_position[profile.source_memory_position];
            local_profile.target_memory_position = local_memory_position[profile.target_memory_position];
            local_profile.boolean_positive_conditions = remap(profile.boolean_positive_conditions, component_analysis.boolean_positions);
            local_profile.boolean_negative_conditions = remap(profile.boolean_negative_conditions, component_analysis.boolean_positions);
            local_profile.numerical_greater_conditions = remap(profile.numerical_greater_conditions, component_analysis.numerical_positions);
            local_profile.numerical_zero_conditions = remap(profile.numerical_zero_conditions, component_analysis.numerical_positions);
            local_profile.boolean_positive_effects = remap(profile.boolean_positive_effects, component_analysis.boolean_positions);
            local_profile.boolean_negative_effects = remap(profile.boolean_negative_effects, component_analysis.boolean_positions);
            local_profile.boolean_unchanged_effects = remap(profile.boolean_unchanged_effects, component_analysis.boolean_positions);
            for (std::size_t local = 0; local < num_numericals; ++local)
                local_profile.numerical_changes[local] = profile.numerical_changes[component_analysis.numerical_positions[local]];
            component_analysis.rule_positions.push_back(rule_position);
            component_analysis.profiles.push_back(std::move(local_profile));
        }

        result.push_back(std::move(component_analysis));
    }
    return result;
}

}  // namespace runir::kr::ps::ext::dl::detail
