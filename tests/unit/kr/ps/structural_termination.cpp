#include "kr/ps/structural_termination/detail.hpp"
#include "kr/ps/structural_termination/scc_refinement_forest.hpp"

#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <tuple>

namespace runir::tests
{
namespace
{
kr::ps::detail::RuleProfile explicit_effects(kr::ps::detail::RuleProfile profile)
{
    profile.boolean_unconstrained_effects &= ~(profile.boolean_positive_effects | profile.boolean_negative_effects | profile.boolean_unchanged_effects);
    profile.numerical_unconstrained_effects &= ~(profile.numerical_increase_effects | profile.numerical_decrease_effects | profile.numerical_unchanged_effects);
    return profile;
}

// Previous per-feature assignment enumeration, retained as an ordered oracle.
std::vector<kr::ps::detail::PolicyEdge> reference_policy_edges(const kr::ps::detail::QualitativePolicy& policy)
{
    auto edges = std::vector<kr::ps::detail::PolicyEdge> {};
    for (std::size_t rule_position = 0; rule_position < policy.rule_profiles.size(); ++rule_position)
    {
        const auto& profile = policy.rule_profiles[rule_position];
        for (std::size_t valuation = 0; valuation < policy.num_valuations(); ++valuation)
        {
            const auto source = valuation * policy.num_memory_states + profile.source_memory_position;
            const auto booleans = kr::ps::detail::vertex_booleans(source, policy);
            const auto numericals = kr::ps::detail::vertex_numericals(source, policy);
            if ((profile.boolean_positive_conditions & ~booleans) || (profile.boolean_negative_conditions & booleans)
                || (profile.numerical_greater_conditions & ~numericals) || (profile.numerical_zero_conditions & numericals)
                || (profile.numerical_decrease_effects & ~numericals))
                continue;

            auto target_booleans = profile.boolean_positive_effects | (profile.boolean_unchanged_effects & booleans);
            auto target_numericals = profile.numerical_increase_effects | (profile.numerical_unchanged_effects & numericals);
            auto free_positions = std::vector<std::pair<bool, std::size_t>> {};
            for (std::size_t position = 0; position < policy.num_booleans; ++position)
                if (profile.boolean_unconstrained_effects & (std::uint64_t { 1 } << position))
                    free_positions.emplace_back(true, position);
            for (std::size_t position = 0; position < policy.num_numericals; ++position)
                if ((profile.numerical_decrease_effects | profile.numerical_unconstrained_effects) & (std::uint64_t { 1 } << position))
                    free_positions.emplace_back(false, position);

            for (std::size_t assignment = 0; assignment < (std::size_t { 1 } << free_positions.size()); ++assignment)
            {
                for (std::size_t free = 0; free < free_positions.size(); ++free)
                {
                    const auto [is_boolean, position] = free_positions[free];
                    auto& target = is_boolean ? target_booleans : target_numericals;
                    target &= ~(std::uint64_t { 1 } << position);
                    target |= std::uint64_t { (assignment >> free) & 1 } << position;
                }
                const auto target_valuation = static_cast<std::size_t>(target_booleans | (target_numericals << policy.num_booleans));
                edges.push_back({ source, target_valuation * policy.num_memory_states + profile.target_memory_position, rule_position });
            }
        }
    }
    return edges;
}

void expect_ordered_edges_match_reference(const kr::ps::detail::QualitativePolicy& policy)
{
    const auto expected = reference_policy_edges(policy);
    const auto actual = kr::ps::detail::build_policy_edges(policy);
    ASSERT_EQ(actual.size(), expected.size());
    for (std::size_t position = 0; position < actual.size(); ++position)
        EXPECT_EQ(std::tie(actual[position].source, actual[position].target, actual[position].rule_position, actual[position].alive),
                  std::tie(expected[position].source, expected[position].target, expected[position].rule_position, expected[position].alive));
}

bool monolithic_sieve_has_cycle(const kr::ps::detail::QualitativePolicy& policy)
{
    auto edges = kr::ps::detail::build_policy_edges(policy);
    return kr::ps::detail::sieve_policy_graph(edges, policy).has_cycle;
}

void expect_hybrid_matches_monolithic(const kr::ps::detail::QualitativePolicy& policy)
{
    EXPECT_EQ(!kr::ps::detail::sieve_policy(policy, 4, true).components.empty(), monolithic_sieve_has_cycle(policy));
}

std::vector<kr::ps::detail::RuleProfile> numerical_rule_universe()
{
    auto result = std::vector<kr::ps::detail::RuleProfile> {};
    for (std::size_t source = 0; source < 2; ++source)
        for (std::size_t target = 0; target < 2; ++target)
            for (std::size_t condition = 0; condition < 3; ++condition)
                for (std::size_t effect = 0; effect < 4; ++effect)
                {
                    auto profile = kr::ps::detail::RuleProfile(0, 1, source, target);
                    if (condition == 1)
                        profile.numerical_greater_conditions |= 1;
                    else if (condition == 2)
                        profile.numerical_zero_conditions |= 1;
                    if (effect == 0)
                        profile.numerical_increase_effects = 1;
                    else if (effect == 1)
                        profile.numerical_decrease_effects = 1;
                    else if (effect == 2)
                        profile.numerical_unchanged_effects = 1;
                    result.push_back(explicit_effects(std::move(profile)));
                }
    return result;
}

std::vector<kr::ps::detail::RuleProfile> boolean_rule_universe()
{
    auto result = std::vector<kr::ps::detail::RuleProfile> {};
    for (std::size_t source = 0; source < 2; ++source)
        for (std::size_t target = 0; target < 2; ++target)
            for (std::size_t condition = 0; condition < 3; ++condition)
                for (std::size_t effect = 0; effect < 4; ++effect)
                {
                    auto profile = kr::ps::detail::RuleProfile(1, 0, source, target);
                    if (condition == 1)
                        profile.boolean_positive_conditions |= 1;
                    else if (condition == 2)
                        profile.boolean_negative_conditions |= 1;
                    if (effect == 1)
                        profile.boolean_positive_effects |= 1;
                    else if (effect == 2)
                        profile.boolean_negative_effects |= 1;
                    else if (effect == 3)
                        profile.boolean_unchanged_effects |= 1;
                    result.push_back(explicit_effects(std::move(profile)));
                }
    return result;
}

void expect_universe_matches_monolithic(const std::vector<kr::ps::detail::RuleProfile>& profiles, std::size_t num_booleans, std::size_t num_numericals)
{
    expect_hybrid_matches_monolithic(kr::ps::detail::QualitativePolicy(2, num_booleans, num_numericals));
    for (std::size_t first = 0; first < profiles.size(); ++first)
    {
        SCOPED_TRACE(::testing::Message() << "first rule " << first);
        auto policy = kr::ps::detail::QualitativePolicy(2, num_booleans, num_numericals);
        policy.rule_profiles.push_back(profiles[first]);
        expect_hybrid_matches_monolithic(policy);
        for (std::size_t second = 0; second < profiles.size(); ++second)
        {
            SCOPED_TRACE(::testing::Message() << "second rule " << second);
            auto pair = kr::ps::detail::QualitativePolicy(2, num_booleans, num_numericals);
            pair.rule_profiles.push_back(profiles[first]);
            pair.rule_profiles.push_back(profiles[second]);
            expect_hybrid_matches_monolithic(pair);
        }
    }
}

}  // namespace

TEST(RunirTests, QualitativePolicyRequiresMemoryState) { EXPECT_THROW((void) kr::ps::detail::QualitativePolicy(0, 0, 0), std::invalid_argument); }

TEST(RunirTests, PackedPolicyEdgesPreserveEverySmallConditionEffectCombination)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 1, 1);
    for (const auto& boolean : boolean_rule_universe())
        for (const auto& numerical : numerical_rule_universe())
        {
            if (boolean.source_memory_position != numerical.source_memory_position || boolean.target_memory_position != numerical.target_memory_position)
                continue;
            auto profile = boolean;
            profile.numerical_greater_conditions = numerical.numerical_greater_conditions;
            profile.numerical_zero_conditions = numerical.numerical_zero_conditions;
            profile.numerical_increase_effects = numerical.numerical_increase_effects;
            profile.numerical_decrease_effects = numerical.numerical_decrease_effects;
            profile.numerical_unchanged_effects = numerical.numerical_unchanged_effects;
            profile.numerical_unconstrained_effects = numerical.numerical_unconstrained_effects;
            policy.rule_profiles.push_back(profile);
        }
    expect_ordered_edges_match_reference(policy);
}

TEST(RunirTests, PackedPolicyEdgesPreserveSparseFreeBitsAndUnchangedBits)
{
    for (std::size_t booleans = 0; booleans <= 4; ++booleans)
        for (std::size_t numericals = 0; numericals <= 4 - booleans; ++numericals)
        {
            auto policy = kr::ps::detail::QualitativePolicy(2, booleans, numericals);
            const auto boolean_mask = (std::uint64_t { 1 } << booleans) - 1;
            const auto numerical_mask = (std::uint64_t { 1 } << numericals) - 1;
            for (std::uint64_t free = 0; free < policy.num_valuations(); ++free)
            {
                auto profile = kr::ps::detail::RuleProfile(booleans, numericals, 0, 1);
                profile.boolean_unconstrained_effects = free & boolean_mask;
                profile.boolean_unchanged_effects = ~free & boolean_mask;
                profile.numerical_unconstrained_effects = free >> booleans;
                profile.numerical_unchanged_effects = ~(free >> booleans) & numerical_mask;
                policy.rule_profiles.push_back(profile);
            }
            expect_ordered_edges_match_reference(policy);
        }
}

TEST(RunirTests, PackedPolicyEdgesRejectFullWidthBeforePackingValuations)
{
    for (const auto booleans : { std::size_t { 0 }, std::size_t { 32 }, std::size_t { 64 } })
    {
        auto policy = kr::ps::detail::QualitativePolicy(1, booleans, 64 - booleans);
        policy.rule_profiles.emplace_back(booleans, 64 - booleans);
        EXPECT_THROW((void) kr::ps::detail::build_policy_edges(policy), std::invalid_argument);
    }
}

TEST(RunirTests, CommonFeaturePositionRejectsUndeclaredFeatureBeforeMaskShift)
{
    struct Feature
    {
        std::size_t index;
        std::size_t get_index() const { return index; }
    };
    auto features = std::array<Feature, 64> {};
    for (std::size_t position = 0; position < features.size(); ++position)
        features[position].index = position;

    EXPECT_EQ(kr::ps::detail::feature_position(features, Feature { 63 }), 63);
    EXPECT_THROW((void) kr::ps::detail::feature_position(features, Feature { 64 }), std::invalid_argument);
    EXPECT_THROW((void) kr::ps::detail::feature_position(std::array<Feature, 0> {}, Feature { 0 }), std::invalid_argument);
}

TEST(RunirTests, CommonSieveEliminatesUnopposedDecrease)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 1);
    auto profile = kr::ps::detail::RuleProfile(0, 1);
    profile.numerical_decrease_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(profile)));

    auto edges = kr::ps::detail::build_policy_edges(policy);
    const auto result = kr::ps::detail::sieve_policy_graph(edges, policy);

    EXPECT_FALSE(result.has_cycle);
    EXPECT_EQ(result.component_of.size(), policy.num_vertices());
}

TEST(RunirTests, CommonSieveEliminatesUnopposedIncrease)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 1);
    auto profile = kr::ps::detail::RuleProfile(0, 1);
    profile.numerical_increase_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(profile)));

    auto edges = kr::ps::detail::build_policy_edges(policy);
    const auto result = kr::ps::detail::sieve_policy_graph(edges, policy);

    EXPECT_FALSE(result.has_cycle);
    EXPECT_TRUE(kr::ps::detail::incomplete_structural_termination(policy).is_terminating());
}

TEST(RunirTests, CommonSieveRetainsOpposingIncreaseDecreaseCycle)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 1);
    auto increase = kr::ps::detail::RuleProfile(0, 1);
    increase.numerical_increase_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(increase)));
    auto decrease = kr::ps::detail::RuleProfile(0, 1);
    decrease.numerical_decrease_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(decrease)));

    auto edges = kr::ps::detail::build_policy_edges(policy);
    const auto result = kr::ps::detail::sieve_policy_graph(edges, policy);

    EXPECT_TRUE(result.has_cycle);
}

TEST(RunirTests, CommonSieveTreatsUnconstrainedNumericalEffectAsBothDirections)
{
    for (const auto progress : { &kr::ps::detail::RuleProfile::numerical_decrease_effects, &kr::ps::detail::RuleProfile::numerical_increase_effects })
    {
        auto policy = kr::ps::detail::QualitativePolicy(1, 0, 1);
        auto profile = kr::ps::detail::RuleProfile(0, 1);
        profile.*progress = 1;
        policy.rule_profiles.push_back(explicit_effects(std::move(profile)));
        policy.rule_profiles.emplace_back(0, 1);

        auto edges = kr::ps::detail::build_policy_edges(policy);
        EXPECT_TRUE(kr::ps::detail::sieve_policy_graph(edges, policy).has_cycle);
        EXPECT_FALSE(kr::ps::detail::incomplete_structural_termination(policy).is_terminating());
    }
}

TEST(RunirTests, CommonSieveHandlesAcyclicMemoryTransition)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 0, 0);
    policy.rule_profiles.emplace_back(0, 0, 0, 1);

    auto edges = kr::ps::detail::build_policy_edges(policy);
    const auto result = kr::ps::detail::sieve_policy_graph(edges, policy);

    ASSERT_EQ(edges.size(), 1);
    EXPECT_EQ(edges.front().source, 0);
    EXPECT_EQ(edges.front().target, 1);
    EXPECT_FALSE(result.has_cycle);
}

TEST(RunirTests, CommonSieveStartsWithIncompleteRuleElimination)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 2);
    auto decrease_y = kr::ps::detail::RuleProfile(0, 2);
    decrease_y.numerical_decrease_effects = 1;
    decrease_y.numerical_unchanged_effects = 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(decrease_y)));
    auto preserve_n = kr::ps::detail::RuleProfile(0, 2);
    preserve_n.numerical_unchanged_effects = 3;
    policy.rule_profiles.push_back(explicit_effects(std::move(preserve_n)));

    const auto result = kr::ps::detail::sieve_policy(policy, 16, true);

    ASSERT_TRUE(monolithic_sieve_has_cycle(policy));
    ASSERT_EQ(result.components.size(), 1);
    EXPECT_EQ(result.components.front().projected.rule_positions, std::vector<std::size_t>({ 1 }));

    const auto without_preprocessing = kr::ps::detail::sieve_policy(policy, 16, false);
    ASSERT_EQ(without_preprocessing.components.size(), 1);
    EXPECT_EQ(without_preprocessing.components.front().projected.rule_positions, std::vector<std::size_t>({ 0, 1 }));
}

TEST(RunirTests, CommonSieveSkipsFeatureLimitWhenIncompleteProcedureTerminates)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 17);
    auto decrease = kr::ps::detail::RuleProfile(0, 17);
    decrease.numerical_unchanged_effects = (std::uint64_t { 1 } << 16) - 1;
    decrease.numerical_decrease_effects = std::uint64_t { 1 } << 16;
    decrease.numerical_greater_conditions = (std::uint64_t { 1 } << 17) - 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(decrease)));

    const auto result = kr::ps::detail::sieve_policy(policy, 16, true);
    EXPECT_TRUE(result.components.empty());
    EXPECT_FALSE(result.scc_feature_positions.has_value());
    EXPECT_THROW((void) kr::ps::detail::sieve_policy(policy, 16, false), std::invalid_argument);
}

TEST(RunirTests, CommonSieveSplitsResidualMemoryGraph)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 0, 1);
    auto decreasing_connector = kr::ps::detail::RuleProfile(0, 1, 0, 1);
    decreasing_connector.numerical_decrease_effects = 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(decreasing_connector)));
    auto return_connector = kr::ps::detail::RuleProfile(0, 1, 1, 0);
    return_connector.numerical_unchanged_effects = 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(return_connector)));
    auto first_loop = kr::ps::detail::RuleProfile(0, 1, 0, 0);
    first_loop.numerical_unchanged_effects = 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(first_loop)));
    auto second_loop = kr::ps::detail::RuleProfile(0, 1, 1, 1);
    second_loop.numerical_unchanged_effects = 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(second_loop)));

    const auto result = kr::ps::detail::sieve_policy(policy, 16, true);

    ASSERT_TRUE(monolithic_sieve_has_cycle(policy));
    ASSERT_EQ(result.components.size(), 2);
    EXPECT_EQ(result.components[0].projected.memory_positions, std::vector<std::size_t>({ 0 }));
    EXPECT_EQ(result.components[0].projected.rule_positions, std::vector<std::size_t>({ 2 }));
    EXPECT_EQ(result.components[1].projected.memory_positions, std::vector<std::size_t>({ 1 }));
    EXPECT_EQ(result.components[1].projected.rule_positions, std::vector<std::size_t>({ 3 }));
}

TEST(RunirTests, CommonSieveProjectsTestedAndChangedFeatures)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 3, 3);
    auto profile = kr::ps::detail::RuleProfile(3, 3);
    profile.boolean_positive_conditions |= 1;
    profile.boolean_unchanged_effects |= 2;
    profile.boolean_positive_effects |= 4;
    profile.numerical_greater_conditions |= 1;
    profile.numerical_unchanged_effects |= 2;
    profile.numerical_increase_effects |= 4;
    policy.rule_profiles.push_back(explicit_effects(std::move(profile)));

    constexpr auto rule_positions = std::array<std::size_t, 1> { 0 };
    const auto projected = kr::ps::detail::project_policy_components(policy, rule_positions);

    ASSERT_EQ(projected.size(), 1);
    EXPECT_EQ(projected.front().boolean_positions, std::vector<std::size_t>({ 0, 2 }));
    EXPECT_EQ(projected.front().numerical_positions, std::vector<std::size_t>({ 0, 2 }));
}

TEST(RunirTests, UnprojectVertexRestoresOriginalFeaturePositions)
{
    const auto policy = kr::ps::detail::QualitativePolicy(4, 5, 6);
    const auto projected = kr::ps::detail::ProjectedPolicyComponent { kr::ps::detail::QualitativePolicy(2, 2, 2), { 3, 1 }, { 4, 1 }, { 5, 2 }, {} };
    for (const auto memory_position : { 0u, 1u })
    {
        // Local Boolean bits 01 and numerical bits 10, followed by the memory position.
        const auto [booleans, numericals] = kr::ps::detail::unproject_vertex(0b1001 * 2 + memory_position, projected, policy);
        EXPECT_EQ(booleans, std::uint64_t { 1 } << 4);
        EXPECT_EQ(numericals, std::uint64_t { 1 } << 2);
    }

    const auto empty = kr::ps::detail::ProjectedPolicyComponent { kr::ps::detail::QualitativePolicy(2, 0, 0), { 3, 1 }, {}, {}, {} };
    const auto [booleans, numericals] = kr::ps::detail::unproject_vertex(1, empty, policy);
    EXPECT_EQ(booleans, 0);
    EXPECT_EQ(numericals, 0);
    const auto [no_booleans, no_numericals] = kr::ps::detail::unproject_vertex(1, empty, kr::ps::detail::QualitativePolicy(4, 0, 0));
    EXPECT_EQ(no_booleans, 0);
    EXPECT_EQ(no_numericals, 0);
}

TEST(RunirTests, CommonSieveAppliesFeatureLimitPerResidualComponent)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 0, 16);
    auto first_loop = kr::ps::detail::RuleProfile(0, 16, 0, 0);
    for (std::size_t position = 0; position < 8; ++position)
    {
        first_loop.numerical_greater_conditions |= std::uint64_t { 1 } << (position);
        first_loop.numerical_unchanged_effects |= std::uint64_t { 1 } << (position);
    }
    policy.rule_profiles.push_back(explicit_effects(std::move(first_loop)));
    auto second_loop = kr::ps::detail::RuleProfile(0, 16, 1, 1);
    for (std::size_t position = 8; position < 16; ++position)
    {
        second_loop.numerical_greater_conditions |= std::uint64_t { 1 } << (position);
        second_loop.numerical_unchanged_effects |= std::uint64_t { 1 } << (position);
    }
    policy.rule_profiles.push_back(explicit_effects(std::move(second_loop)));

    const auto result = kr::ps::detail::sieve_policy(policy, 14, true);

    ASSERT_EQ(result.components.size(), 2);
    EXPECT_EQ(result.components[0].projected.policy.num_numericals, 8);
    EXPECT_EQ(result.components[1].projected.policy.num_numericals, 8);
    ASSERT_TRUE(result.scc_feature_positions);
    ASSERT_EQ(result.scc_feature_positions->size(), 2);
    EXPECT_EQ(result.scc_feature_positions->front().numerical_positions, std::vector<std::size_t>({ 0, 1, 2, 3, 4, 5, 6, 7 }));
    EXPECT_EQ(result.scc_feature_positions->back().numerical_positions, std::vector<std::size_t>({ 8, 9, 10, 11, 12, 13, 14, 15 }));
}

TEST(RunirTests, CommonSieveRejectsOversizedResidualComponent)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 15);
    auto loop = kr::ps::detail::RuleProfile(0, 15);
    loop.numerical_greater_conditions = (std::uint64_t { 1 } << 15) - 1;
    loop.numerical_unchanged_effects = loop.numerical_greater_conditions;
    policy.rule_profiles.push_back(explicit_effects(std::move(loop)));

    EXPECT_THROW((void) kr::ps::detail::sieve_policy(policy, 14, true), std::invalid_argument);
}

TEST(RunirTests, CommonHybridSieveMatchesMonolithicForBoundedRuleUniverses)
{
    expect_universe_matches_monolithic(numerical_rule_universe(), 0, 1);
    expect_universe_matches_monolithic(boolean_rule_universe(), 1, 0);
}

TEST(RunirTests, CommonIncompleteSieveUsesDisconnectedMemoryComponents)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 1, 0);
    auto to_true = kr::ps::detail::RuleProfile(1, 0, 0, 0);
    to_true.boolean_negative_conditions |= 1;
    to_true.boolean_positive_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_true)));
    auto to_false = kr::ps::detail::RuleProfile(1, 0, 1, 1);
    to_false.boolean_positive_conditions |= 1;
    to_false.boolean_negative_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_false)));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_TRUE(result.surviving_rules.empty());
}

TEST(RunirTests, CommonIncompleteSieveCanUseGlobalOpponentScope)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 1, 0);
    auto to_true = kr::ps::detail::RuleProfile(1, 0, 0, 0);
    to_true.boolean_negative_conditions |= 1;
    to_true.boolean_positive_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_true)));
    auto to_false = kr::ps::detail::RuleProfile(1, 0, 1, 1);
    to_false.boolean_positive_conditions |= 1;
    to_false.boolean_negative_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_false)));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy, false);

    EXPECT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 2);
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 1 }));
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 0 }));
}

TEST(RunirTests, SccRefinementForestInheritsMarksAcrossSplits)
{
    const auto component_of = std::vector<std::size_t> { 0, 0, 0 };
    auto forest = kr::ps::detail::SccRefinementForest(component_of, 1, 1, 1);
    const auto root = forest.roots().front();
    forest.mark_numerical(root, 0, 0);
    forest.mark_numerical(root, 0, 2);
    forest.mark_numerical(root, 0, 0);

    const auto partitions = std::vector<std::vector<std::size_t>> { { 0 }, { 1, 2 } };
    const auto children = forest.split(root, partitions);
    forest.mark_boolean(children[1], 0, 1);

    const auto first_marks = forest.effective_marks(children[0]);
    EXPECT_TRUE((first_marks.numericals & 1) != 0);
    EXPECT_FALSE((first_marks.booleans & 1) != 0);
    EXPECT_EQ(first_marks.numerical_witnessing_rule_positions[0], std::vector<std::size_t>({ 0, 2 }));

    const auto second_marks = forest.effective_marks(children[1]);
    EXPECT_TRUE((second_marks.numericals & 1) != 0);
    EXPECT_TRUE((second_marks.booleans & 1) != 0);
    EXPECT_EQ(second_marks.numerical_witnessing_rule_positions[0], std::vector<std::size_t>({ 0, 2 }));
    EXPECT_EQ(second_marks.boolean_witnessing_rule_positions[0], std::vector<std::size_t>({ 1 }));
}

TEST(RunirTests, CommonIncompleteSieveOnlyEliminatesAcyclicMemoryRuleWithSccScope)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 0, 0);
    policy.rule_profiles.emplace_back(0, 0, 0, 1);

    const auto scoped = kr::ps::detail::incomplete_structural_termination(policy);
    const auto global = kr::ps::detail::incomplete_structural_termination(policy, false);

    EXPECT_TRUE(scoped.is_terminating());
    EXPECT_TRUE(scoped.surviving_rules.empty());
    EXPECT_FALSE(global.is_terminating());
    ASSERT_EQ(global.surviving_rules.size(), 1);
    EXPECT_EQ(global.surviving_rules.front().rule_position, 0);
}

TEST(RunirTests, CommonIncompleteSieveDoesNotLeakMarksAcrossMemoryComponents)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 0, 2);

    auto mark_y = kr::ps::detail::RuleProfile(0, 2, 0, 0);
    mark_y.numerical_greater_conditions |= 1;
    mark_y.numerical_decrease_effects = 1;
    mark_y.numerical_unchanged_effects = 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(mark_y)));

    auto decrease_n = kr::ps::detail::RuleProfile(0, 2, 1, 1);
    decrease_n.numerical_greater_conditions |= 1;
    decrease_n.numerical_greater_conditions |= 2;
    decrease_n.numerical_decrease_effects = 2;
    decrease_n.numerical_unchanged_effects = 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(decrease_n)));

    auto increase_n = kr::ps::detail::RuleProfile(0, 2, 1, 1);
    increase_n.numerical_zero_conditions |= 1;
    increase_n.numerical_zero_conditions |= 2;
    increase_n.numerical_increase_effects = 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(increase_n)));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 2);
    EXPECT_EQ(result.surviving_rules[0].rule_position, 1);
    EXPECT_EQ(result.surviving_rules[1].rule_position, 2);
    ASSERT_EQ(result.surviving_rules[0].blocking_reasons.size(), 1);
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 2 }));
}

TEST(RunirTests, CommonIncompleteSieveInheritsMarksWhenMemoryComponentSplits)
{
    for (const auto y_position : { 1u, 63u })
    {
        SCOPED_TRACE(y_position);
        const auto y = std::uint64_t { 1 } << y_position;
        constexpr auto n = std::uint64_t { 1 };
        auto policy = kr::ps::detail::QualitativePolicy(2, 0, y_position + 1);

        auto mark_y = kr::ps::detail::RuleProfile(0, y_position + 1, 0, 1);
        mark_y.numerical_greater_conditions = y;
        mark_y.numerical_decrease_effects = y;
        mark_y.numerical_unchanged_effects = n;
        policy.rule_profiles.push_back(explicit_effects(std::move(mark_y)));

        auto return_to_m0 = kr::ps::detail::RuleProfile(0, y_position + 1, 1, 0);
        return_to_m0.numerical_unchanged_effects = y | n;
        policy.rule_profiles.push_back(explicit_effects(std::move(return_to_m0)));

        auto decrease_n = kr::ps::detail::RuleProfile(0, y_position + 1, 1, 1);
        decrease_n.numerical_greater_conditions = y | n;
        decrease_n.numerical_decrease_effects = n;
        decrease_n.numerical_unchanged_effects = y;
        policy.rule_profiles.push_back(explicit_effects(std::move(decrease_n)));

        auto increase_n = kr::ps::detail::RuleProfile(0, y_position + 1, 1, 1);
        increase_n.numerical_zero_conditions = y | n;
        increase_n.numerical_increase_effects = n;
        increase_n.numerical_unchanged_effects = y;
        policy.rule_profiles.push_back(explicit_effects(std::move(increase_n)));

        const auto result = kr::ps::detail::incomplete_structural_termination(policy);

        EXPECT_TRUE(result.is_terminating());
        EXPECT_TRUE(result.surviving_rules.empty());
    }
}

TEST(RunirTests, CommonIncompleteSievePrefersMarkingWitnessOverR3)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 1, 3);

    auto mark_y = kr::ps::detail::RuleProfile(1, 3);
    mark_y.numerical_greater_conditions |= 1;
    mark_y.boolean_unchanged_effects |= 1;
    mark_y.numerical_decrease_effects = 1;
    mark_y.numerical_unchanged_effects = 6;
    policy.rule_profiles.push_back(explicit_effects(std::move(mark_y)));

    auto mark_z = kr::ps::detail::RuleProfile(1, 3);
    mark_z.numerical_greater_conditions |= 1;
    mark_z.numerical_greater_conditions |= 2;
    mark_z.numerical_greater_conditions |= 4;
    mark_z.boolean_unchanged_effects |= 1;
    mark_z.numerical_decrease_effects = 6;
    mark_z.numerical_unchanged_effects = 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(mark_z)));

    auto increase_x = kr::ps::detail::RuleProfile(1, 3);
    increase_x.numerical_zero_conditions |= 1;
    increase_x.boolean_unchanged_effects |= 1;
    increase_x.numerical_increase_effects = 2;
    increase_x.numerical_unchanged_effects = 5;
    policy.rule_profiles.push_back(explicit_effects(std::move(increase_x)));

    auto to_true = kr::ps::detail::RuleProfile(1, 3);
    to_true.boolean_negative_conditions |= 1;
    to_true.numerical_greater_conditions |= 4;
    to_true.boolean_positive_effects |= 1;
    to_true.numerical_unchanged_effects = 7;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_true)));

    auto to_false = kr::ps::detail::RuleProfile(1, 3);
    to_false.boolean_positive_conditions |= 1;
    to_false.numerical_zero_conditions |= 4;
    to_false.boolean_negative_effects |= 1;
    to_false.numerical_unchanged_effects = 7;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_false)));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_TRUE(result.surviving_rules.empty());
}

TEST(RunirTests, CommonIncompleteSieveMarksParentBeforeMemorySplit)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 2, 0);
    auto connector = kr::ps::detail::RuleProfile(2, 0, 0, 1);
    connector.boolean_unchanged_effects = 3;
    policy.rule_profiles.push_back(explicit_effects(std::move(connector)));

    auto mark_y = kr::ps::detail::RuleProfile(2, 0, 1, 0);
    mark_y.boolean_negative_conditions |= 1;
    mark_y.boolean_positive_effects |= 1;
    mark_y.boolean_unchanged_effects |= 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(mark_y)));

    auto first_to_true = kr::ps::detail::RuleProfile(2, 0, 0, 0);
    first_to_true.boolean_positive_conditions |= 1;
    first_to_true.boolean_negative_conditions |= 2;
    first_to_true.boolean_unchanged_effects |= 1;
    first_to_true.boolean_positive_effects |= 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(first_to_true)));

    auto first_to_false = kr::ps::detail::RuleProfile(2, 0, 0, 0);
    first_to_false.boolean_negative_conditions |= 1;
    first_to_false.boolean_positive_conditions |= 2;
    first_to_false.boolean_unchanged_effects |= 1;
    first_to_false.boolean_negative_effects |= 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(first_to_false)));

    auto second_to_true = kr::ps::detail::RuleProfile(2, 0, 1, 1);
    second_to_true.boolean_negative_conditions |= 1;
    second_to_true.boolean_negative_conditions |= 2;
    second_to_true.boolean_unchanged_effects |= 1;
    second_to_true.boolean_positive_effects |= 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(second_to_true)));

    auto second_to_false = kr::ps::detail::RuleProfile(2, 0, 1, 1);
    second_to_false.boolean_positive_conditions |= 1;
    second_to_false.boolean_positive_conditions |= 2;
    second_to_false.boolean_unchanged_effects |= 1;
    second_to_false.boolean_negative_effects |= 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(second_to_false)));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_TRUE(result.surviving_rules.empty());
}

TEST(RunirTests, CommonIncompleteSieveDoesNotMarkCrossSccRule)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 2, 0);

    auto cross = kr::ps::detail::RuleProfile(2, 0, 0, 1);
    cross.boolean_negative_conditions |= 1;
    cross.boolean_positive_effects |= 1;
    cross.boolean_unchanged_effects |= 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(cross)));

    auto opposing_loop = kr::ps::detail::RuleProfile(2, 0, 1, 1);
    opposing_loop.boolean_positive_conditions |= 1;
    opposing_loop.boolean_negative_effects |= 1;
    opposing_loop.boolean_unchanged_effects |= 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(opposing_loop)));

    auto to_true = kr::ps::detail::RuleProfile(2, 0, 0, 0);
    to_true.boolean_positive_conditions |= 1;
    to_true.boolean_negative_conditions |= 2;
    to_true.boolean_unchanged_effects |= 1;
    to_true.boolean_positive_effects |= 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_true)));

    auto to_false = kr::ps::detail::RuleProfile(2, 0, 0, 0);
    to_false.boolean_negative_conditions |= 1;
    to_false.boolean_positive_conditions |= 2;
    to_false.boolean_unchanged_effects |= 1;
    to_false.boolean_negative_effects |= 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_false)));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 2);
    EXPECT_EQ(result.surviving_rules[0].rule_position, 2);
    EXPECT_EQ(result.surviving_rules[1].rule_position, 3);
}

TEST(RunirTests, CommonIncompleteSieveTreatsUnconstrainedBooleanEffectAsOpponent)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 1, 0);
    auto to_true = kr::ps::detail::RuleProfile(1, 0);
    to_true.boolean_negative_conditions |= 1;
    to_true.boolean_positive_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_true)));
    policy.rule_profiles.emplace_back(1, 0);

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 2);
    ASSERT_EQ(result.surviving_rules.front().blocking_reasons.size(), 1);
    EXPECT_EQ(result.surviving_rules.front().blocking_reasons.front().feature_kind, kr::ps::detail::IncompletePolicyResult::FeatureKind::BOOLEAN);
    EXPECT_EQ(result.surviving_rules.front().blocking_reasons.front().opposing_rule_positions, std::vector<std::size_t>({ 1 }));
}

TEST(RunirTests, CommonIncompleteSieveHasNoFeatureLimit)
{
    for (const auto position : { 16u, 63u })
        for (const auto progress : { &kr::ps::detail::RuleProfile::numerical_decrease_effects, &kr::ps::detail::RuleProfile::numerical_increase_effects })
        {
            SCOPED_TRACE(position);
            auto policy = kr::ps::detail::QualitativePolicy(1, 0, position + 1);
            auto rule = kr::ps::detail::RuleProfile(0, position + 1);
            rule.*progress = std::uint64_t { 1 } << position;
            policy.rule_profiles.push_back(explicit_effects(rule));

            EXPECT_TRUE(kr::ps::detail::incomplete_structural_termination(policy).is_terminating());
        }
}

TEST(RunirTests, CommonIncompleteSievePreservesOriginalPositionsAndReasonOrder)
{
    using FeatureKind = kr::ps::detail::IncompletePolicyResult::FeatureKind;

    auto policy = kr::ps::detail::QualitativePolicy(1, 1, 2);
    auto eliminated = kr::ps::detail::RuleProfile(1, 2);
    eliminated.numerical_unchanged_effects = 1;
    eliminated.numerical_decrease_effects = 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(eliminated)));

    auto first_survivor = kr::ps::detail::RuleProfile(1, 2);
    first_survivor.boolean_negative_conditions |= 1;
    first_survivor.boolean_positive_effects |= 1;
    first_survivor.numerical_unchanged_effects = 2;
    first_survivor.numerical_decrease_effects = 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(first_survivor)));

    auto second_survivor = kr::ps::detail::RuleProfile(1, 2);
    second_survivor.boolean_positive_conditions |= 1;
    second_survivor.boolean_negative_effects |= 1;
    second_survivor.numerical_unchanged_effects = 2;
    second_survivor.numerical_increase_effects = 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(second_survivor)));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    ASSERT_EQ(result.surviving_rules.size(), 2);
    EXPECT_EQ(result.surviving_rules[0].rule_position, 1);
    EXPECT_EQ(result.surviving_rules[1].rule_position, 2);

    ASSERT_EQ(result.surviving_rules[0].blocking_reasons.size(), 2);
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[0].feature_kind, FeatureKind::NUMERICAL);
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[0].feature_position, 0);
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 2 }));
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[1].feature_kind, FeatureKind::BOOLEAN);
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[1].feature_position, 0);
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[1].opposing_rule_positions, std::vector<std::size_t>({ 2 }));

    ASSERT_EQ(result.surviving_rules[1].blocking_reasons.size(), 2);
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[0].feature_kind, FeatureKind::NUMERICAL);
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[0].feature_position, 0);
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 1 }));
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[1].feature_kind, FeatureKind::BOOLEAN);
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[1].opposing_rule_positions, std::vector<std::size_t>({ 1 }));
}

TEST(RunirTests, CommonIncompleteSieveDoesNotMarkFeatureAfterR3Elimination)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 1, 2);

    auto mark_y = kr::ps::detail::RuleProfile(1, 2);
    mark_y.numerical_greater_conditions |= 1;
    mark_y.boolean_unchanged_effects |= 1;
    mark_y.numerical_decrease_effects = 1;
    mark_y.numerical_unchanged_effects = 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(mark_y)));

    auto r3_eliminated = kr::ps::detail::RuleProfile(1, 2);
    r3_eliminated.numerical_greater_conditions |= 1;
    r3_eliminated.numerical_greater_conditions |= 2;
    r3_eliminated.boolean_unchanged_effects |= 1;
    r3_eliminated.numerical_decrease_effects = 2;
    r3_eliminated.numerical_unchanged_effects = 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(r3_eliminated)));

    auto to_true = kr::ps::detail::RuleProfile(1, 2);
    to_true.boolean_negative_conditions |= 1;
    to_true.numerical_zero_conditions |= 1;
    to_true.numerical_zero_conditions |= 2;
    to_true.boolean_positive_effects |= 1;
    to_true.numerical_increase_effects = 2;
    to_true.numerical_unchanged_effects = 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_true)));

    auto to_false = kr::ps::detail::RuleProfile(1, 2);
    to_false.boolean_positive_conditions |= 1;
    to_false.numerical_zero_conditions |= 1;
    to_false.numerical_greater_conditions |= 2;
    to_false.boolean_negative_effects |= 1;
    to_false.numerical_decrease_effects = 2;
    to_false.numerical_unchanged_effects = 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_false)));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 2);
    EXPECT_EQ(result.surviving_rules[0].rule_position, 2);
    EXPECT_EQ(result.surviving_rules[1].rule_position, 3);
    ASSERT_EQ(result.surviving_rules[0].blocking_reasons.size(), 2);
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 3 }));
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[1].opposing_rule_positions, std::vector<std::size_t>({ 3 }));
    ASSERT_EQ(result.surviving_rules[1].blocking_reasons.size(), 2);
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 2 }));
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[1].opposing_rule_positions, std::vector<std::size_t>({ 2 }));

    EXPECT_EQ(!kr::ps::detail::sieve_policy(policy, 16, true).components.empty(), monolithic_sieve_has_cycle(policy));
}

TEST(RunirTests, CommonSieveReportsProjectedSccBeforeCompleteFiltering)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 1);
    auto decrease = kr::ps::detail::RuleProfile(0, 1);
    decrease.numerical_decrease_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(decrease)));

    const auto result = kr::ps::detail::sieve_policy(policy, 16, false);

    EXPECT_TRUE(result.components.empty());
    ASSERT_TRUE(result.scc_feature_positions);
    ASSERT_EQ(result.scc_feature_positions->size(), 1);
    EXPECT_TRUE(result.scc_feature_positions->front().boolean_positions.empty());
    EXPECT_EQ(result.scc_feature_positions->front().numerical_positions, std::vector<std::size_t>({ 0 }));
}

TEST(RunirTests, CommonSieveReportsEngagedEmptySccsWhenCompleteStageRuns)
{
    const auto result = kr::ps::detail::sieve_policy(kr::ps::detail::QualitativePolicy(1, 0, 0), 16, false);

    EXPECT_TRUE(result.components.empty());
    ASSERT_TRUE(result.scc_feature_positions);
    EXPECT_TRUE(result.scc_feature_positions->empty());
}

}  // namespace runir::tests
