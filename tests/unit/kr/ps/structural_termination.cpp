#include "kr/ps/structural_termination/detail.hpp"

#include <gtest/gtest.h>

namespace runir::tests
{

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

TEST(RunirTests, CommonIncompleteSieveIgnoresDisconnectedMemoryStates)
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

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 2);
    ASSERT_EQ(result.surviving_rules[0].blocking_reasons.size(), 1);
    ASSERT_EQ(result.surviving_rules[1].blocking_reasons.size(), 1);
    EXPECT_EQ(result.surviving_rules[0].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 1 }));
    EXPECT_EQ(result.surviving_rules[1].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 0 }));
}

TEST(RunirTests, CommonIncompleteSieveRetainsCrossMemoryRule)
{
    auto policy = kr::ps::detail::QualitativePolicy(2, 0, 0);
    policy.rule_profiles.emplace_back(0, 0, 0, 1);

    const auto result = kr::ps::detail::incomplete_structural_termination(policy);

    ASSERT_FALSE(result.is_terminating());
    ASSERT_EQ(result.surviving_rules.size(), 1);
    EXPECT_EQ(result.surviving_rules.front().rule_position, 0);
    EXPECT_TRUE(result.surviving_rules.front().blocking_reasons.empty());
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
}

}  // namespace runir::tests
