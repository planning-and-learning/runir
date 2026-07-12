#include "kr/ps/structural_termination/detail.hpp"

#include <array>
#include <gtest/gtest.h>

namespace runir::tests
{

namespace
{

bool monolithic_sieve_has_cycle(const kr::ps::detail::QualitativePolicy& policy)
{
    auto edges = kr::ps::detail::build_policy_edges(policy);
    return kr::ps::detail::sieve_policy_graph(edges, policy).has_cycle;
}

void expect_hybrid_matches_monolithic(const kr::ps::detail::QualitativePolicy& policy)
{
    EXPECT_EQ(!kr::ps::detail::sieve_policy(policy, 4, true).empty(), monolithic_sieve_has_cycle(policy));
}

std::vector<kr::ps::detail::RuleProfile> numerical_rule_universe()
{
    using NumericalChange = kr::ps::dl::NumericalChange;
    constexpr auto changes = std::array { NumericalChange::INCREASES, NumericalChange::DECREASES, NumericalChange::UNCHANGED, NumericalChange::UNCONSTRAINED };
    auto result = std::vector<kr::ps::detail::RuleProfile> {};
    for (std::size_t source = 0; source < 2; ++source)
        for (std::size_t target = 0; target < 2; ++target)
            for (std::size_t condition = 0; condition < 3; ++condition)
                for (const auto change : changes)
                {
                    auto profile = kr::ps::detail::RuleProfile(0, 1, source, target);
                    if (condition == 1)
                        profile.numerical_greater_conditions.set(0);
                    else if (condition == 2)
                        profile.numerical_zero_conditions.set(0);
                    profile.numerical_changes[0] = change;
                    result.push_back(std::move(profile));
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
                        profile.boolean_positive_conditions.set(0);
                    else if (condition == 2)
                        profile.boolean_negative_conditions.set(0);
                    if (effect == 1)
                        profile.boolean_positive_effects.set(0);
                    else if (effect == 2)
                        profile.boolean_negative_effects.set(0);
                    else if (effect == 3)
                        profile.boolean_unchanged_effects.set(0);
                    result.push_back(std::move(profile));
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

TEST(RunirTests, CommonSieveEliminatesUnopposedDecrease)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 1);
    auto profile = kr::ps::detail::RuleProfile(0, 1);
    profile.numerical_changes[0] = kr::ps::dl::NumericalChange::DECREASES;
    policy.rule_profiles.push_back(std::move(profile));

    auto edges = kr::ps::detail::build_policy_edges(policy);
    const auto result = kr::ps::detail::sieve_policy_graph(edges, policy);

    EXPECT_FALSE(result.has_cycle);
    EXPECT_EQ(result.component_of.size(), policy.num_vertices());
}

TEST(RunirTests, CommonSieveRetainsOpposingIncreaseDecreaseCycle)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 1);
    auto increase = kr::ps::detail::RuleProfile(0, 1);
    increase.numerical_changes[0] = kr::ps::dl::NumericalChange::INCREASES;
    policy.rule_profiles.push_back(std::move(increase));
    auto decrease = kr::ps::detail::RuleProfile(0, 1);
    decrease.numerical_changes[0] = kr::ps::dl::NumericalChange::DECREASES;
    policy.rule_profiles.push_back(std::move(decrease));

    auto edges = kr::ps::detail::build_policy_edges(policy);
    const auto result = kr::ps::detail::sieve_policy_graph(edges, policy);

    EXPECT_TRUE(result.has_cycle);
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
    using NumericalChange = kr::ps::dl::NumericalChange;

    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 2);
    auto decrease_y = kr::ps::detail::RuleProfile(0, 2);
    decrease_y.numerical_changes = { NumericalChange::DECREASES, NumericalChange::UNCHANGED };
    policy.rule_profiles.push_back(std::move(decrease_y));
    auto increase_n = kr::ps::detail::RuleProfile(0, 2);
    increase_n.numerical_changes = { NumericalChange::UNCHANGED, NumericalChange::INCREASES };
    policy.rule_profiles.push_back(std::move(increase_n));

    const auto result = kr::ps::detail::sieve_policy(policy, 16, true);

    ASSERT_TRUE(monolithic_sieve_has_cycle(policy));
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result.front().projected.rule_positions, std::vector<std::size_t>({ 1 }));

    const auto without_preprocessing = kr::ps::detail::sieve_policy(policy, 16, false);
    ASSERT_EQ(without_preprocessing.size(), 1);
    EXPECT_EQ(without_preprocessing.front().projected.rule_positions, std::vector<std::size_t>({ 0, 1 }));
}

TEST(RunirTests, CommonSieveSkipsFeatureLimitWhenIncompleteProcedureTerminates)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 17);
    auto decrease = kr::ps::detail::RuleProfile(0, 17);
    decrease.numerical_changes.assign(17, kr::ps::dl::NumericalChange::UNCHANGED);
    decrease.numerical_changes[16] = kr::ps::dl::NumericalChange::DECREASES;
    policy.rule_profiles.push_back(std::move(decrease));

    EXPECT_TRUE(kr::ps::detail::sieve_policy(policy, 16, true).empty());
    EXPECT_THROW((void) kr::ps::detail::sieve_policy(policy, 16, false), std::invalid_argument);
}

TEST(RunirTests, CommonSieveSplitsResidualMemoryGraph)
{
    using NumericalChange = kr::ps::dl::NumericalChange;

    auto policy = kr::ps::detail::QualitativePolicy(2, 0, 1);
    auto decreasing_connector = kr::ps::detail::RuleProfile(0, 1, 0, 1);
    decreasing_connector.numerical_changes = { NumericalChange::DECREASES };
    policy.rule_profiles.push_back(std::move(decreasing_connector));
    auto return_connector = kr::ps::detail::RuleProfile(0, 1, 1, 0);
    return_connector.numerical_changes = { NumericalChange::UNCHANGED };
    policy.rule_profiles.push_back(std::move(return_connector));
    auto first_loop = kr::ps::detail::RuleProfile(0, 1, 0, 0);
    first_loop.numerical_changes = { NumericalChange::UNCHANGED };
    policy.rule_profiles.push_back(std::move(first_loop));
    auto second_loop = kr::ps::detail::RuleProfile(0, 1, 1, 1);
    second_loop.numerical_changes = { NumericalChange::UNCHANGED };
    policy.rule_profiles.push_back(std::move(second_loop));

    const auto result = kr::ps::detail::sieve_policy(policy, 16, true);

    ASSERT_TRUE(monolithic_sieve_has_cycle(policy));
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].projected.memory_positions, std::vector<std::size_t>({ 0 }));
    EXPECT_EQ(result[0].projected.rule_positions, std::vector<std::size_t>({ 2 }));
    EXPECT_EQ(result[1].projected.memory_positions, std::vector<std::size_t>({ 1 }));
    EXPECT_EQ(result[1].projected.rule_positions, std::vector<std::size_t>({ 3 }));
}

TEST(RunirTests, CommonSieveProjectsResidualFeatures)
{
    using NumericalChange = kr::ps::dl::NumericalChange;

    auto policy = kr::ps::detail::QualitativePolicy(1, 3, 2);
    auto profile = kr::ps::detail::RuleProfile(3, 2);
    profile.boolean_positive_conditions.set(0);
    profile.boolean_unchanged_effects.set(1);
    profile.numerical_changes[1] = NumericalChange::UNCHANGED;
    policy.rule_profiles.push_back(std::move(profile));

    const auto result = kr::ps::detail::sieve_policy(policy, 16, true);

    ASSERT_EQ(!result.empty(), monolithic_sieve_has_cycle(policy));
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result.front().projected.boolean_positions, std::vector<std::size_t>({ 0, 1 }));
    EXPECT_EQ(result.front().projected.numerical_positions, std::vector<std::size_t>({ 1 }));
}

TEST(RunirTests, CommonSieveAppliesFeatureLimitPerResidualComponent)
{
    using NumericalChange = kr::ps::dl::NumericalChange;

    auto policy = kr::ps::detail::QualitativePolicy(2, 0, 16);
    auto first_loop = kr::ps::detail::RuleProfile(0, 16, 0, 0);
    for (std::size_t position = 0; position < 8; ++position)
        first_loop.numerical_changes[position] = NumericalChange::UNCHANGED;
    policy.rule_profiles.push_back(std::move(first_loop));
    auto second_loop = kr::ps::detail::RuleProfile(0, 16, 1, 1);
    for (std::size_t position = 8; position < 16; ++position)
        second_loop.numerical_changes[position] = NumericalChange::UNCHANGED;
    policy.rule_profiles.push_back(std::move(second_loop));

    const auto result = kr::ps::detail::sieve_policy(policy, 14, true);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].projected.policy.num_numericals, 8);
    EXPECT_EQ(result[1].projected.policy.num_numericals, 8);
}

TEST(RunirTests, CommonSieveRejectsOversizedResidualComponent)
{
    using NumericalChange = kr::ps::dl::NumericalChange;

    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 15);
    auto loop = kr::ps::detail::RuleProfile(0, 15);
    loop.numerical_changes.assign(15, NumericalChange::UNCHANGED);
    policy.rule_profiles.push_back(std::move(loop));

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
    to_true.boolean_negative_conditions.set(0);
    to_true.boolean_positive_effects.set(0);
    policy.rule_profiles.push_back(std::move(to_true));
    auto to_false = kr::ps::detail::RuleProfile(1, 0, 1, 1);
    to_false.boolean_positive_conditions.set(0);
    to_false.boolean_negative_effects.set(0);
    policy.rule_profiles.push_back(std::move(to_false));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_TRUE(result.surviving_rules.empty());
}

TEST(RunirTests, CommonIncompleteSieveEliminatesAcyclicMemoryRule)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 0, 0);
    policy.rule_profiles.emplace_back(0, 0, 0, 1);

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    EXPECT_TRUE(result.is_terminating());
    EXPECT_TRUE(result.surviving_rules.empty());
}

TEST(RunirTests, CommonIncompleteSieveDoesNotLeakMarksAcrossMemoryComponents)
{
    using NumericalChange = kr::ps::dl::NumericalChange;

    auto policy = kr::ps::detail::QualitativePolicy(2, 0, 2);

    auto mark_y = kr::ps::detail::RuleProfile(0, 2, 0, 0);
    mark_y.numerical_greater_conditions.set(0);
    mark_y.numerical_changes = { NumericalChange::DECREASES, NumericalChange::UNCHANGED };
    policy.rule_profiles.push_back(std::move(mark_y));

    auto decrease_n = kr::ps::detail::RuleProfile(0, 2, 1, 1);
    decrease_n.numerical_greater_conditions.set(0);
    decrease_n.numerical_greater_conditions.set(1);
    decrease_n.numerical_changes = { NumericalChange::UNCHANGED, NumericalChange::DECREASES };
    policy.rule_profiles.push_back(std::move(decrease_n));

    auto increase_n = kr::ps::detail::RuleProfile(0, 2, 1, 1);
    increase_n.numerical_zero_conditions.set(0);
    increase_n.numerical_zero_conditions.set(1);
    increase_n.numerical_changes = { NumericalChange::UNCHANGED, NumericalChange::INCREASES };
    policy.rule_profiles.push_back(std::move(increase_n));

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
    using NumericalChange = kr::ps::dl::NumericalChange;

    auto policy = kr::ps::detail::QualitativePolicy(2, 0, 2);

    auto mark_y = kr::ps::detail::RuleProfile(0, 2, 0, 1);
    mark_y.numerical_greater_conditions.set(0);
    mark_y.numerical_changes = { NumericalChange::DECREASES, NumericalChange::UNCHANGED };
    policy.rule_profiles.push_back(std::move(mark_y));

    auto return_to_m0 = kr::ps::detail::RuleProfile(0, 2, 1, 0);
    return_to_m0.numerical_changes = { NumericalChange::UNCHANGED, NumericalChange::UNCHANGED };
    policy.rule_profiles.push_back(std::move(return_to_m0));

    auto decrease_n = kr::ps::detail::RuleProfile(0, 2, 1, 1);
    decrease_n.numerical_greater_conditions.set(0);
    decrease_n.numerical_greater_conditions.set(1);
    decrease_n.numerical_changes = { NumericalChange::UNCHANGED, NumericalChange::DECREASES };
    policy.rule_profiles.push_back(std::move(decrease_n));

    auto increase_n = kr::ps::detail::RuleProfile(0, 2, 1, 1);
    increase_n.numerical_zero_conditions.set(0);
    increase_n.numerical_zero_conditions.set(1);
    increase_n.numerical_changes = { NumericalChange::UNCHANGED, NumericalChange::INCREASES };
    policy.rule_profiles.push_back(std::move(increase_n));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 1);
    EXPECT_EQ(result.surviving_rules.front().rule_position, 3);
}

TEST(RunirTests, CommonIncompleteSieveTreatsUnconstrainedBooleanEffectAsOpponent)
{
    auto policy = kr::ps::detail::QualitativePolicy(1, 1, 0);
    auto to_true = kr::ps::detail::RuleProfile(1, 0);
    to_true.boolean_negative_conditions.set(0);
    to_true.boolean_positive_effects.set(0);
    policy.rule_profiles.push_back(std::move(to_true));
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
    auto policy = kr::ps::detail::QualitativePolicy(1, 0, 17);
    auto decrease = kr::ps::detail::RuleProfile(0, 17);
    decrease.numerical_changes[16] = kr::ps::dl::NumericalChange::DECREASES;
    policy.rule_profiles.push_back(std::move(decrease));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    EXPECT_TRUE(result.is_terminating());
}

TEST(RunirTests, CommonIncompleteSievePreservesOriginalPositionsAndReasonOrder)
{
    using NumericalChange = kr::ps::dl::NumericalChange;
    using FeatureKind = kr::ps::detail::IncompletePolicyResult::FeatureKind;

    auto policy = kr::ps::detail::QualitativePolicy(1, 1, 2);
    auto eliminated = kr::ps::detail::RuleProfile(1, 2);
    eliminated.numerical_changes.assign(2, NumericalChange::UNCHANGED);
    eliminated.numerical_changes[1] = NumericalChange::DECREASES;
    policy.rule_profiles.push_back(std::move(eliminated));

    auto first_survivor = kr::ps::detail::RuleProfile(1, 2);
    first_survivor.boolean_negative_conditions.set(0);
    first_survivor.boolean_positive_effects.set(0);
    first_survivor.numerical_changes.assign(2, NumericalChange::UNCHANGED);
    first_survivor.numerical_changes[0] = NumericalChange::DECREASES;
    policy.rule_profiles.push_back(std::move(first_survivor));

    auto second_survivor = kr::ps::detail::RuleProfile(1, 2);
    second_survivor.boolean_positive_conditions.set(0);
    second_survivor.boolean_negative_effects.set(0);
    second_survivor.numerical_changes.assign(2, NumericalChange::UNCHANGED);
    second_survivor.numerical_changes[0] = NumericalChange::INCREASES;
    policy.rule_profiles.push_back(std::move(second_survivor));

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

    ASSERT_EQ(result.surviving_rules[1].blocking_reasons.size(), 1);
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[0].feature_kind, FeatureKind::BOOLEAN);
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 1 }));
}

TEST(RunirTests, CommonIncompleteSieveDoesNotMarkFeatureAfterR3Elimination)
{
    using NumericalChange = kr::ps::dl::NumericalChange;

    auto policy = kr::ps::detail::QualitativePolicy(1, 1, 2);

    auto mark_y = kr::ps::detail::RuleProfile(1, 2);
    mark_y.numerical_greater_conditions.set(0);
    mark_y.boolean_unchanged_effects.set(0);
    mark_y.numerical_changes = { NumericalChange::DECREASES, NumericalChange::UNCHANGED };
    policy.rule_profiles.push_back(std::move(mark_y));

    auto r3_eliminated = kr::ps::detail::RuleProfile(1, 2);
    r3_eliminated.numerical_greater_conditions.set(0);
    r3_eliminated.numerical_greater_conditions.set(1);
    r3_eliminated.boolean_unchanged_effects.set(0);
    r3_eliminated.numerical_changes = { NumericalChange::UNCHANGED, NumericalChange::DECREASES };
    policy.rule_profiles.push_back(std::move(r3_eliminated));

    auto to_true = kr::ps::detail::RuleProfile(1, 2);
    to_true.boolean_negative_conditions.set(0);
    to_true.numerical_zero_conditions.set(0);
    to_true.numerical_zero_conditions.set(1);
    to_true.boolean_positive_effects.set(0);
    to_true.numerical_changes = { NumericalChange::UNCHANGED, NumericalChange::INCREASES };
    policy.rule_profiles.push_back(std::move(to_true));

    auto to_false = kr::ps::detail::RuleProfile(1, 2);
    to_false.boolean_positive_conditions.set(0);
    to_false.numerical_zero_conditions.set(0);
    to_false.numerical_greater_conditions.set(1);
    to_false.boolean_negative_effects.set(0);
    to_false.numerical_changes = { NumericalChange::UNCHANGED, NumericalChange::DECREASES };
    policy.rule_profiles.push_back(std::move(to_false));

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 2);
    EXPECT_EQ(result.surviving_rules[0].rule_position, 2);
    EXPECT_EQ(result.surviving_rules[1].rule_position, 3);
    ASSERT_EQ(result.surviving_rules[0].blocking_reasons.size(), 1);
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 3 }));
    ASSERT_EQ(result.surviving_rules[1].blocking_reasons.size(), 2);
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 2 }));
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[1].opposing_rule_positions, std::vector<std::size_t>({ 2 }));

    EXPECT_EQ(!kr::ps::detail::sieve_policy(policy, 16, true).empty(), monolithic_sieve_has_cycle(policy));
}

}  // namespace runir::tests
