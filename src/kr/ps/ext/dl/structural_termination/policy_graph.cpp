#include "detail.hpp"

#include <stdexcept>

namespace runir::kr::ps::ext::dl::detail
{

/// Vertex ids encode valuation * num_memory_states + memory position; within
/// a valuation, Boolean feature i is at bit i and numerical feature j
/// at bit num_booleans + j.
boost::dynamic_bitset<> vertex_booleans(std::size_t vertex, std::size_t num_memory_states, std::size_t num_booleans)
{
    const auto valuation = vertex / num_memory_states;
    auto values = boost::dynamic_bitset<>(num_booleans);
    for (std::size_t position = 0; position < num_booleans; ++position)
        values.set(position, (valuation >> position) & std::size_t { 1 });
    return values;
}

boost::dynamic_bitset<> vertex_numericals(std::size_t vertex, std::size_t num_memory_states, std::size_t num_booleans, std::size_t num_numericals)
{
    const auto valuation = vertex / num_memory_states;
    auto values = boost::dynamic_bitset<>(num_numericals);
    for (std::size_t position = 0; position < num_numericals; ++position)
        values.set(position, (valuation >> (num_booleans + position)) & std::size_t { 1 });
    return values;
}

inline std::size_t
make_vertex(const boost::dynamic_bitset<>& booleans, const boost::dynamic_bitset<>& numericals, std::size_t num_memory_states, std::size_t memory_position)
{
    auto valuation = std::size_t { 0 };
    for (std::size_t position = 0; position < booleans.size(); ++position)
        if (booleans.test(position))
            valuation |= std::size_t { 1 } << position;
    for (std::size_t position = 0; position < numericals.size(); ++position)
        if (numericals.test(position))
            valuation |= std::size_t { 1 } << (booleans.size() + position);
    return valuation * num_memory_states + memory_position;
}

/// Enumerate the extended policy graph edges of one rule by calling back
/// with (source vertex, target vertex).
template<typename Callback>
void enumerate_rule_edges(const ModuleRuleProfile& profile,
                          std::size_t num_memory_states,
                          std::size_t num_booleans,
                          std::size_t num_numericals,
                          Callback&& callback)
{
    const auto num_valuations = std::size_t { 1 } << (num_booleans + num_numericals);

    for (std::size_t source_valuation = 0; source_valuation < num_valuations; ++source_valuation)
    {
        const auto source = source_valuation * num_memory_states + profile.source_memory_position;
        const auto source_booleans = vertex_booleans(source, num_memory_states, num_booleans);
        const auto source_numericals = vertex_numericals(source, num_memory_states, num_booleans, num_numericals);

        // Conditions.
        if (!profile.boolean_positive_conditions.is_subset_of(source_booleans))
            continue;
        if (profile.boolean_negative_conditions.intersects(source_booleans))
            continue;
        if (!profile.numerical_greater_conditions.is_subset_of(source_numericals))
            continue;
        if (profile.numerical_zero_conditions.intersects(source_numericals))
            continue;

        // Effects: compute forced target values and the free positions.
        auto target_booleans = profile.boolean_positive_effects | (profile.boolean_unchanged_effects & source_booleans);
        const auto fixed_booleans = profile.boolean_positive_effects | profile.boolean_negative_effects | profile.boolean_unchanged_effects;

        auto target_numericals = boost::dynamic_bitset<>(num_numericals);
        auto fixed_numericals = boost::dynamic_bitset<>(num_numericals);
        auto decreases_unsatisfiable = false;
        for (std::size_t position = 0; position < num_numericals; ++position)
        {
            switch (profile.numerical_changes[position])
            {
                case NumericalChange::INCREASES:
                {
                    target_numericals.set(position);
                    fixed_numericals.set(position);
                    break;
                }
                case NumericalChange::DECREASES:
                {
                    if (!source_numericals.test(position))
                        decreases_unsatisfiable = true;
                    break;
                }
                case NumericalChange::UNCHANGED:
                {
                    target_numericals.set(position, source_numericals.test(position));
                    fixed_numericals.set(position);
                    break;
                }
                case NumericalChange::UNCONSTRAINED:
                    break;
            }
        }
        if (decreases_unsatisfiable)
            continue;

        auto free_positions = std::vector<std::pair<bool, std::size_t>> {};  // (is_boolean, position)
        for (std::size_t position = 0; position < num_booleans; ++position)
            if (!fixed_booleans.test(position))
                free_positions.emplace_back(true, position);
        for (std::size_t position = 0; position < num_numericals; ++position)
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
            callback(source, make_vertex(target_booleans, target_numericals, num_memory_states, profile.target_memory_position));
        }
    }
}

std::vector<ModulePolicyEdge>
build_policy_edges(const std::vector<ModuleRuleProfile>& profiles, std::size_t num_memory_states, std::size_t num_booleans, std::size_t num_numericals)
{
    auto edges = std::vector<ModulePolicyEdge> {};
    for (std::size_t rule_position = 0; rule_position < profiles.size(); ++rule_position)
        enumerate_rule_edges(profiles[rule_position],
                             num_memory_states,
                             num_booleans,
                             num_numericals,
                             [&](std::size_t source, std::size_t target) { edges.push_back(ModulePolicyEdge { source, target, rule_position }); });
    return edges;
}

std::vector<ModulePolicyEdge> build_policy_edges(const ModuleAnalysis& analysis)
{
    const auto num_memory_states = analysis.memory_states.size();
    if (num_memory_states == 0)
        return {};

    const auto num_booleans = analysis.features.booleans.size();
    const auto num_numericals = analysis.features.numericals.size();
    if (num_booleans + num_numericals > 14)
        throw std::invalid_argument("structural_termination: too many features; the policy graph has 2^|features| * |memory| vertices.");

    return build_policy_edges(analysis.profiles, num_memory_states, num_booleans, num_numericals);
}

}  // namespace runir::kr::ps::ext::dl::detail
