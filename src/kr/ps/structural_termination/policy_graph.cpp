#include "detail.hpp"

#include <utility>

namespace runir::kr::ps::detail
{

boost::dynamic_bitset<> vertex_booleans(std::size_t vertex, const QualitativePolicy& policy)
{
    const auto valuation = vertex / policy.num_memory_states;
    auto values = boost::dynamic_bitset<>(policy.num_booleans);
    for (std::size_t position = 0; position < policy.num_booleans; ++position)
        values.set(position, (valuation >> position) & std::size_t { 1 });
    return values;
}

boost::dynamic_bitset<> vertex_numericals(std::size_t vertex, const QualitativePolicy& policy)
{
    const auto valuation = vertex / policy.num_memory_states;
    auto values = boost::dynamic_bitset<>(policy.num_numericals);
    for (std::size_t position = 0; position < policy.num_numericals; ++position)
        values.set(position, (valuation >> (policy.num_booleans + position)) & std::size_t { 1 });
    return values;
}

namespace
{

std::size_t
make_vertex(const boost::dynamic_bitset<>& booleans, const boost::dynamic_bitset<>& numericals, std::size_t memory_position, const QualitativePolicy& policy)
{
    auto valuation = std::size_t { 0 };
    for (std::size_t position = 0; position < booleans.size(); ++position)
        if (booleans.test(position))
            valuation |= std::size_t { 1 } << position;
    for (std::size_t position = 0; position < numericals.size(); ++position)
        if (numericals.test(position))
            valuation |= std::size_t { 1 } << (booleans.size() + position);
    return valuation * policy.num_memory_states + memory_position;
}

template<typename Callback>
void enumerate_rule_edges(const RuleProfile& profile, const QualitativePolicy& policy, Callback&& callback)
{
    for (std::size_t source_valuation = 0; source_valuation < policy.num_valuations(); ++source_valuation)
    {
        const auto source = source_valuation * policy.num_memory_states + profile.source_memory_position;
        const auto source_booleans = vertex_booleans(source, policy);
        const auto source_numericals = vertex_numericals(source, policy);

        if (!profile.boolean_positive_conditions.is_subset_of(source_booleans))
            continue;
        if (profile.boolean_negative_conditions.intersects(source_booleans))
            continue;
        if (!profile.numerical_greater_conditions.is_subset_of(source_numericals))
            continue;
        if (profile.numerical_zero_conditions.intersects(source_numericals))
            continue;

        auto target_booleans = profile.boolean_positive_effects | (profile.boolean_unchanged_effects & source_booleans);
        const auto fixed_booleans = profile.boolean_positive_effects | profile.boolean_negative_effects | profile.boolean_unchanged_effects;

        auto target_numericals = boost::dynamic_bitset<>(policy.num_numericals);
        auto fixed_numericals = boost::dynamic_bitset<>(policy.num_numericals);
        auto decreases_unsatisfiable = false;
        for (std::size_t position = 0; position < policy.num_numericals; ++position)
        {
            switch (profile.numerical_changes[position])
            {
                case dl::NumericalChange::INCREASES:
                    target_numericals.set(position);
                    fixed_numericals.set(position);
                    break;
                case dl::NumericalChange::DECREASES:
                    if (!source_numericals.test(position))
                        decreases_unsatisfiable = true;
                    break;
                case dl::NumericalChange::UNCHANGED:
                    target_numericals.set(position, source_numericals.test(position));
                    fixed_numericals.set(position);
                    break;
                case dl::NumericalChange::UNCONSTRAINED:
                    break;
            }
        }
        if (decreases_unsatisfiable)
            continue;

        auto free_positions = std::vector<std::pair<bool, std::size_t>> {};
        for (std::size_t position = 0; position < policy.num_booleans; ++position)
            if (!fixed_booleans.test(position))
                free_positions.emplace_back(true, position);
        for (std::size_t position = 0; position < policy.num_numericals; ++position)
            if (!fixed_numericals.test(position))
                free_positions.emplace_back(false, position);

        for (std::size_t assignment = 0; assignment < (std::size_t { 1 } << free_positions.size()); ++assignment)
        {
            for (std::size_t free = 0; free < free_positions.size(); ++free)
            {
                const auto [is_boolean, position] = free_positions[free];
                const auto value = static_cast<bool>((assignment >> free) & std::size_t { 1 });
                (is_boolean ? target_booleans : target_numericals).set(position, value);
            }
            callback(source, make_vertex(target_booleans, target_numericals, profile.target_memory_position, policy));
        }
    }
}

void append_rule_edges(const QualitativePolicy& policy, std::size_t rule_position, std::vector<PolicyEdge>& edges)
{
    enumerate_rule_edges(policy.rule_profiles[rule_position],
                         policy,
                         [&](std::size_t source, std::size_t target) { edges.push_back(PolicyEdge { source, target, rule_position }); });
}

std::vector<std::size_t> positions(const boost::dynamic_bitset<>& selected)
{
    auto result = std::vector<std::size_t> {};
    for (std::size_t position = 0; position < selected.size(); ++position)
        if (selected.test(position))
            result.push_back(position);
    return result;
}

std::vector<std::size_t> relevant_booleans(const QualitativePolicy& policy, std::span<const std::size_t> rule_positions)
{
    auto selected = boost::dynamic_bitset<>(policy.num_booleans);
    for (const auto rule_position : rule_positions)
    {
        const auto& profile = policy.rule_profiles[rule_position];
        selected |=
            profile.boolean_positive_conditions | profile.boolean_negative_conditions | profile.boolean_positive_effects | profile.boolean_negative_effects;
    }
    return positions(selected);
}

std::vector<std::size_t> relevant_numericals(const QualitativePolicy& policy, std::span<const std::size_t> rule_positions)
{
    auto selected = boost::dynamic_bitset<>(policy.num_numericals);
    for (const auto rule_position : rule_positions)
    {
        const auto& profile = policy.rule_profiles[rule_position];
        selected |= profile.numerical_greater_conditions | profile.numerical_zero_conditions;
        for (std::size_t position = 0; position < profile.numerical_changes.size(); ++position)
        {
            const auto change = profile.numerical_changes[position];
            if (change == dl::NumericalChange::INCREASES || change == dl::NumericalChange::DECREASES)
                selected.set(position);
        }
    }
    return positions(selected);
}

RuleProfile project_profile(const RuleProfile& profile,
                            const std::vector<std::size_t>& memory_position_map,
                            const std::vector<std::size_t>& boolean_positions,
                            const std::vector<std::size_t>& numerical_positions)
{
    auto projected = RuleProfile(boolean_positions.size(),
                                 numerical_positions.size(),
                                 memory_position_map[profile.source_memory_position],
                                 memory_position_map[profile.target_memory_position]);
    for (std::size_t local = 0; local < boolean_positions.size(); ++local)
    {
        const auto global = boolean_positions[local];
        projected.boolean_positive_conditions.set(local, profile.boolean_positive_conditions.test(global));
        projected.boolean_negative_conditions.set(local, profile.boolean_negative_conditions.test(global));
        projected.boolean_positive_effects.set(local, profile.boolean_positive_effects.test(global));
        projected.boolean_negative_effects.set(local, profile.boolean_negative_effects.test(global));
        projected.boolean_unchanged_effects.set(local, profile.boolean_unchanged_effects.test(global));
    }
    for (std::size_t local = 0; local < numerical_positions.size(); ++local)
    {
        const auto global = numerical_positions[local];
        projected.numerical_greater_conditions.set(local, profile.numerical_greater_conditions.test(global));
        projected.numerical_zero_conditions.set(local, profile.numerical_zero_conditions.test(global));
        projected.numerical_changes[local] = profile.numerical_changes[global];
    }
    return projected;
}

}  // namespace

std::vector<PolicyEdge> build_policy_edges(const QualitativePolicy& policy)
{
    auto edges = std::vector<PolicyEdge> {};
    for (std::size_t rule_position = 0; rule_position < policy.rule_profiles.size(); ++rule_position)
        append_rule_edges(policy, rule_position, edges);
    return edges;
}

std::vector<ProjectedPolicyComponent> project_policy_components(const QualitativePolicy& policy, std::span<const std::size_t> rule_positions)
{
    auto memory_edges = std::vector<PolicyEdge> {};
    memory_edges.reserve(rule_positions.size());
    for (const auto rule_position : rule_positions)
    {
        const auto& profile = policy.rule_profiles[rule_position];
        memory_edges.push_back(PolicyEdge { profile.source_memory_position, profile.target_memory_position, rule_position });
    }

    const auto components = find_strong_components(memory_edges, policy.num_memory_states);
    auto memories_by_component = std::vector<std::vector<std::size_t>>(components.count);
    for (std::size_t memory_position = 0; memory_position < policy.num_memory_states; ++memory_position)
        memories_by_component[components.component_of[memory_position]].push_back(memory_position);

    auto rules_by_component = std::vector<std::vector<std::size_t>>(components.count);
    for (const auto rule_position : rule_positions)
    {
        const auto& profile = policy.rule_profiles[rule_position];
        const auto source_component = components.component_of[profile.source_memory_position];
        if (source_component == components.component_of[profile.target_memory_position])
            rules_by_component[source_component].push_back(rule_position);
    }

    auto result = std::vector<ProjectedPolicyComponent> {};
    auto emitted = std::vector<bool>(components.count, false);
    for (std::size_t memory_position = 0; memory_position < policy.num_memory_states; ++memory_position)
    {
        const auto component = components.component_of[memory_position];
        if (emitted[component] || rules_by_component[component].empty())
            continue;
        emitted[component] = true;

        auto memory_positions = std::move(memories_by_component[component]);
        auto rule_positions_ = std::move(rules_by_component[component]);
        auto boolean_positions = relevant_booleans(policy, rule_positions_);
        auto numerical_positions = relevant_numericals(policy, rule_positions_);
        auto projected_policy = QualitativePolicy(memory_positions.size(), boolean_positions.size(), numerical_positions.size());

        auto memory_position_map = std::vector<std::size_t>(policy.num_memory_states);
        for (std::size_t local = 0; local < memory_positions.size(); ++local)
            memory_position_map[memory_positions[local]] = local;
        for (const auto rule_position : rule_positions_)
            projected_policy.rule_profiles.push_back(
                project_profile(policy.rule_profiles[rule_position], memory_position_map, boolean_positions, numerical_positions));

        result.push_back(ProjectedPolicyComponent { std::move(projected_policy),
                                                    std::move(memory_positions),
                                                    std::move(boolean_positions),
                                                    std::move(numerical_positions),
                                                    std::move(rule_positions_) });
    }
    return result;
}

}  // namespace runir::kr::ps::detail
