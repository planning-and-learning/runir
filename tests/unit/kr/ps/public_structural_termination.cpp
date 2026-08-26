#include <algorithm>
#include <gtest/gtest.h>
#include <limits>
#include <runir/kr/ps/dl/structural_termination.hpp>
#include <vector>

namespace runir::tests
{

namespace
{

namespace dl = kr::ps::dl;

dl::RuleProfile unchanged_boolean_loop(std::size_t num_booleans)
{
    auto rule = dl::RuleProfile(num_booleans, 0);
    rule.boolean_positive_conditions.set();
    rule.boolean_unchanged_effects.set();
    return rule;
}

}  // namespace

TEST(RunirTests, PublicStructuralTerminationReportsFailureFrontier)
{
    auto terminating = dl::QualitativePolicy(1, 0, 1);
    auto decrease = dl::RuleProfile(0, 1);
    decrease.numerical_greater_conditions.set(0);
    decrease.numerical_changes[0] = dl::NumericalChange::DECREASES;
    terminating.rule_profiles.push_back(std::move(decrease));
    EXPECT_TRUE(dl::structural_termination(terminating).is_terminating());

    auto policy = dl::QualitativePolicy(2, 1, 0);
    auto isolated_witness = dl::RuleProfile(1, 0, 0, 0);
    isolated_witness.boolean_negative_conditions.set(0);
    isolated_witness.boolean_positive_effects.set(0);
    policy.rule_profiles.push_back(std::move(isolated_witness));

    auto to_true = dl::RuleProfile(1, 0, 1, 1);
    to_true.boolean_negative_conditions.set(0);
    to_true.boolean_positive_effects.set(0);
    policy.rule_profiles.push_back(std::move(to_true));

    auto to_false = dl::RuleProfile(1, 0, 1, 1);
    to_false.boolean_positive_conditions.set(0);
    to_false.boolean_negative_effects.set(0);
    policy.rule_profiles.push_back(std::move(to_false));

    const auto result = dl::structural_termination(policy);

    EXPECT_EQ(result.status, dl::StructuralTerminationStatus::NON_TERMINATING);
    ASSERT_TRUE(result.incomplete_frontier.has_value());
    const auto& frontier = *result.incomplete_frontier;
    ASSERT_EQ(frontier.surviving_rules.size(), 2);
    EXPECT_EQ(frontier.surviving_rules[0].rule_position, 1);
    ASSERT_EQ(frontier.surviving_rules[0].blocking_reasons.size(), 1);
    EXPECT_EQ(frontier.surviving_rules[0].blocking_reasons[0].feature_kind, dl::IncompletePolicyResult::FeatureKind::BOOLEAN);
    EXPECT_EQ(frontier.surviving_rules[0].blocking_reasons[0].feature_position, 0);
    EXPECT_EQ(frontier.surviving_rules[0].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 2 }));
    EXPECT_EQ(frontier.surviving_rules[1].rule_position, 2);
    ASSERT_EQ(frontier.surviving_rules[1].blocking_reasons.size(), 1);
    EXPECT_EQ(frontier.surviving_rules[1].blocking_reasons[0].feature_kind, dl::IncompletePolicyResult::FeatureKind::BOOLEAN);
    EXPECT_EQ(frontier.surviving_rules[1].blocking_reasons[0].feature_position, 0);
    EXPECT_EQ(frontier.surviving_rules[1].blocking_reasons[0].opposing_rule_positions, std::vector<std::size_t>({ 1 }));

    ASSERT_EQ(frontier.residual_scopes.size(), 2);
    const auto scope_zero =
        std::ranges::find(frontier.residual_scopes, std::vector<std::size_t> { 0 }, &dl::IncompletePolicyResult::ResidualMemoryScope::memory_positions);
    const auto scope_one =
        std::ranges::find(frontier.residual_scopes, std::vector<std::size_t> { 1 }, &dl::IncompletePolicyResult::ResidualMemoryScope::memory_positions);
    ASSERT_NE(scope_zero, frontier.residual_scopes.end());
    ASSERT_NE(scope_one, frontier.residual_scopes.end());
    ASSERT_EQ(scope_zero->marked_features.size(), 1);
    EXPECT_EQ(scope_zero->marked_features[0].feature_kind, dl::IncompletePolicyResult::FeatureKind::BOOLEAN);
    EXPECT_EQ(scope_zero->marked_features[0].feature_position, 0);
    EXPECT_EQ(scope_zero->marked_features[0].witnessing_rule_positions, std::vector<std::size_t>({ 0 }));
    EXPECT_TRUE(scope_one->marked_features.empty());

    ASSERT_EQ(result.counterexample_components.size(), 1);
    const auto& counterexample = result.counterexample_components.front();
    EXPECT_EQ(counterexample.memory_positions, std::vector<std::size_t>({ 1 }));
    EXPECT_EQ(counterexample.boolean_positions, std::vector<std::size_t>({ 0 }));
    EXPECT_TRUE(counterexample.numerical_positions.empty());
    ASSERT_EQ(counterexample.vertices.size(), 2);
    ASSERT_EQ(counterexample.edges.size(), 2);
    for (const auto& vertex : counterexample.vertices)
        EXPECT_EQ(vertex.memory_position, 1);
    for (const auto& edge : counterexample.edges)
    {
        const auto source = counterexample.vertices.at(edge.source_vertex_position).boolean_values.test(0);
        const auto target = counterexample.vertices.at(edge.target_vertex_position).boolean_values.test(0);
        if (edge.rule_position == 1)
        {
            EXPECT_FALSE(source);
            EXPECT_TRUE(target);
        }
        else
        {
            EXPECT_EQ(edge.rule_position, 2);
            EXPECT_TRUE(source);
            EXPECT_FALSE(target);
        }
    }

    const auto without_incomplete = dl::structural_termination(policy, dl::default_max_features, false);
    EXPECT_FALSE(without_incomplete.is_terminating());
    EXPECT_FALSE(without_incomplete.incomplete_frontier.has_value());

    auto disjoint_cycles = dl::QualitativePolicy(1, 1, 0);
    auto false_loop = dl::RuleProfile(1, 0);
    false_loop.boolean_negative_conditions.set(0);
    false_loop.boolean_unchanged_effects.set(0);
    disjoint_cycles.rule_profiles.push_back(std::move(false_loop));
    auto true_loop = dl::RuleProfile(1, 0);
    true_loop.boolean_positive_conditions.set(0);
    true_loop.boolean_unchanged_effects.set(0);
    disjoint_cycles.rule_profiles.push_back(std::move(true_loop));

    const auto split = dl::structural_termination(disjoint_cycles);
    ASSERT_EQ(split.counterexample_components.size(), 2);
    for (std::size_t position = 0; position < split.counterexample_components.size(); ++position)
    {
        const auto& component = split.counterexample_components[position];
        ASSERT_EQ(component.vertices.size(), 1);
        ASSERT_EQ(component.edges.size(), 1);
        EXPECT_EQ(component.edges[0].rule_position, position);
        EXPECT_EQ(component.vertices[0].boolean_values.test(0), static_cast<bool>(position));
    }
}

TEST(RunirTests, PublicStructuralTerminationValidatesProfilesAndEnumeration)
{
    EXPECT_THROW((void) dl::QualitativePolicy(0, 0, 0), std::invalid_argument);

    auto bad_endpoint = dl::QualitativePolicy(1, 0, 0);
    bad_endpoint.rule_profiles.emplace_back(0, 0, 0, 1);
    EXPECT_THROW((void) dl::structural_termination(bad_endpoint), std::invalid_argument);

    auto bad_size = dl::QualitativePolicy(1, 1, 0);
    bad_size.rule_profiles.emplace_back(0, 0);
    EXPECT_THROW((void) dl::structural_termination(bad_size), std::invalid_argument);

    auto contradictory = dl::QualitativePolicy(1, 1, 0);
    contradictory.rule_profiles.emplace_back(1, 0);
    contradictory.rule_profiles.back().boolean_positive_conditions.set(0);
    contradictory.rule_profiles.back().boolean_negative_conditions.set(0);
    EXPECT_THROW((void) dl::structural_termination(contradictory), std::invalid_argument);

    auto contradictory_effect = dl::QualitativePolicy(1, 1, 0);
    contradictory_effect.rule_profiles.emplace_back(1, 0);
    contradictory_effect.rule_profiles.back().boolean_positive_effects.set(0);
    contradictory_effect.rule_profiles.back().boolean_unchanged_effects.set(0);
    EXPECT_THROW((void) dl::structural_termination(contradictory_effect), std::invalid_argument);

    auto contradictory_numerical = dl::QualitativePolicy(1, 0, 1);
    contradictory_numerical.rule_profiles.emplace_back(0, 1);
    contradictory_numerical.rule_profiles.back().numerical_greater_conditions.set(0);
    contradictory_numerical.rule_profiles.back().numerical_zero_conditions.set(0);
    EXPECT_THROW((void) dl::structural_termination(contradictory_numerical), std::invalid_argument);

    auto impossible_transition = dl::QualitativePolicy(1, 0, 1);
    impossible_transition.rule_profiles.emplace_back(0, 1);
    impossible_transition.rule_profiles.back().numerical_zero_conditions.set(0);
    impossible_transition.rule_profiles.back().numerical_changes[0] = dl::NumericalChange::DECREASES;
    EXPECT_TRUE(dl::structural_termination(impossible_transition, dl::default_max_features, false).is_terminating());

    auto over_limit = dl::QualitativePolicy(1, 1, 0);
    over_limit.rule_profiles.push_back(unchanged_boolean_loop(1));
    EXPECT_THROW((void) dl::structural_termination(over_limit, 0, false), std::invalid_argument);

    constexpr auto digits = std::numeric_limits<std::size_t>::digits;
    auto shift_overflow = dl::QualitativePolicy(1, digits, 0);
    shift_overflow.rule_profiles.push_back(unchanged_boolean_loop(digits));
    EXPECT_THROW((void) dl::structural_termination(shift_overflow, digits, false), std::invalid_argument);

    auto multiplication_overflow = dl::QualitativePolicy(2, digits - 1, 0);
    auto forward = unchanged_boolean_loop(digits - 1);
    forward.target_memory_position = 1;
    multiplication_overflow.rule_profiles.push_back(std::move(forward));
    auto backward = unchanged_boolean_loop(digits - 1);
    backward.source_memory_position = 1;
    multiplication_overflow.rule_profiles.push_back(std::move(backward));
    EXPECT_THROW((void) dl::structural_termination(multiplication_overflow, digits - 1, false), std::invalid_argument);
}

}  // namespace runir::tests
