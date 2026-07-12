#include "detail.hpp"

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

}  // namespace

std::vector<PolicyEdge> build_policy_edges(const QualitativePolicy& policy)
{
    auto edges = std::vector<PolicyEdge> {};
    for (std::size_t rule_position = 0; rule_position < policy.rule_profiles.size(); ++rule_position)
        enumerate_rule_edges(policy.rule_profiles[rule_position],
                             policy,
                             [&](std::size_t source, std::size_t target) { edges.push_back(PolicyEdge { source, target, rule_position }); });
    return edges;
}

}  // namespace runir::kr::ps::detail
