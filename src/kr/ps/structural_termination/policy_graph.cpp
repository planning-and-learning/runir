#include "detail.hpp"
#include "runir/graphs/declarations.hpp"

#include <bit>
#include <limits>
#include <utility>
#include <yggdrasil/containers/dynamic_bitset.hpp>

namespace runir::kr::ps::detail
{

// Internal Sieve IDs encode both feature categories in one valuation:
//   valuation = booleans | (numericals << num_booleans)
//   vertex = valuation * num_memory_states + memory_position
// Dividing by num_memory_states leaves [numerical bits | Boolean bits].
// Booleans occupy the low num_booleans bits; numericals require shifting past
// them. Returned vertex labels store separate masks, unlike this packed ID.
std::uint64_t vertex_booleans(std::size_t vertex, const QualitativePolicy& policy)
{
    if (policy.num_booleans == 0)
        return 0;
    return (vertex / policy.num_memory_states) & ygg::BitsetSpan<const std::uint64_t>::last_mask(policy.num_booleans);
}

std::uint64_t vertex_numericals(std::size_t vertex, const QualitativePolicy& policy)
{
    if (policy.num_numericals == 0)
        return 0;
    return static_cast<std::uint64_t>(vertex / policy.num_memory_states) >> policy.num_booleans;
}

namespace
{

void append_rule_edges(const QualitativePolicy& policy, std::size_t rule_position, std::vector<PolicyEdge>& edges)
{
    const auto& profile = policy.rule_profiles[rule_position];
    for (std::size_t source_valuation = 0; source_valuation < policy.num_valuations(); ++source_valuation)
    {
        const auto source = source_valuation * policy.num_memory_states + profile.source_memory_position;
        const auto source_booleans = vertex_booleans(source, policy);
        const auto source_numericals = vertex_numericals(source, policy);

        if ((profile.boolean_positive_conditions & ~source_booleans) || (profile.boolean_negative_conditions & source_booleans)
            || (profile.numerical_greater_conditions & ~source_numericals) || (profile.numerical_zero_conditions & source_numericals)
            || (profile.numerical_decrease_effects & ~source_numericals))
            continue;

        const auto target_booleans = profile.boolean_positive_effects | (profile.boolean_unchanged_effects & source_booleans);
        // A decrease requires a positive source (checked above), but may end at
        // either zero or positive, so it remains free in the target valuation.
        const auto target_numericals = profile.numerical_increase_effects | (profile.numerical_unchanged_effects & source_numericals);
        const auto fixed_target = target_booleans | (target_numericals << policy.num_booleans);
        const auto free_numericals = profile.numerical_decrease_effects | profile.numerical_unconstrained_effects;
        const auto free = profile.boolean_unconstrained_effects | (free_numericals << policy.num_booleans);

        // Unsigned subtraction visits subsets in ascending valuation order,
        // including the single empty assignment when no target bits are free.
        for (auto subset = std::uint64_t { 0 };; subset = (subset - free) & free)
        {
            if (edges.size() == std::numeric_limits<graphs::EdgeIndex>::max())
                throw std::invalid_argument("structural_termination: an expanded policy graph has too many edges");
            const auto target = static_cast<std::size_t>(fixed_target | subset) * policy.num_memory_states + profile.target_memory_position;
            edges.push_back(PolicyEdge { source, target, rule_position });
            if (subset == free)
                break;
        }
    }
}

std::vector<std::size_t> positions(std::uint64_t selected)
{
    auto result = std::vector<std::size_t> {};
    for (; selected; selected &= selected - 1)
        result.push_back(std::countr_zero(selected));
    return result;
}

std::vector<std::size_t> relevant_booleans(const QualitativePolicy& policy, std::span<const std::size_t> rule_positions)
{
    auto selected = std::uint64_t { 0 };
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
    auto selected = std::uint64_t { 0 };
    for (const auto rule_position : rule_positions)
    {
        const auto& profile = policy.rule_profiles[rule_position];
        selected |=
            profile.numerical_greater_conditions | profile.numerical_zero_conditions | profile.numerical_increase_effects | profile.numerical_decrease_effects;
    }
    return positions(selected);
}

std::uint64_t project_mask(std::uint64_t mask, const std::vector<std::size_t>& positions)
{
    auto projected = std::uint64_t { 0 };
    for (std::size_t local = 0; local < positions.size(); ++local)
        projected |= ((mask >> positions[local]) & 1) << local;
    return projected;
}

std::uint64_t unproject_mask(std::uint64_t mask, const std::vector<std::size_t>& positions)
{
    auto unprojected = std::uint64_t { 0 };
    for (std::size_t local = 0; local < positions.size(); ++local)
        unprojected |= ((mask >> local) & 1) << positions[local];
    return unprojected;
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
    projected.boolean_positive_conditions = project_mask(profile.boolean_positive_conditions, boolean_positions);
    projected.boolean_negative_conditions = project_mask(profile.boolean_negative_conditions, boolean_positions);
    projected.numerical_greater_conditions = project_mask(profile.numerical_greater_conditions, numerical_positions);
    projected.numerical_zero_conditions = project_mask(profile.numerical_zero_conditions, numerical_positions);
    projected.boolean_positive_effects = project_mask(profile.boolean_positive_effects, boolean_positions);
    projected.boolean_negative_effects = project_mask(profile.boolean_negative_effects, boolean_positions);
    projected.boolean_unchanged_effects = project_mask(profile.boolean_unchanged_effects, boolean_positions);
    projected.boolean_unconstrained_effects = project_mask(profile.boolean_unconstrained_effects, boolean_positions);
    projected.numerical_increase_effects = project_mask(profile.numerical_increase_effects, numerical_positions);
    projected.numerical_decrease_effects = project_mask(profile.numerical_decrease_effects, numerical_positions);
    projected.numerical_unchanged_effects = project_mask(profile.numerical_unchanged_effects, numerical_positions);
    projected.numerical_unconstrained_effects = project_mask(profile.numerical_unconstrained_effects, numerical_positions);
    return projected;
}

}  // namespace

std::pair<std::uint64_t, std::uint64_t> unproject_vertex(std::size_t vertex, const ProjectedPolicyComponent& projected)
{
    return { unproject_mask(vertex_booleans(vertex, projected.policy), projected.boolean_positions),
             unproject_mask(vertex_numericals(vertex, projected.policy), projected.numerical_positions) };
}

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
