#include <algorithm>
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <runir/kr/ps/dl/structural_termination.hpp>
#include <utility>
#include <vector>

namespace runir::tests
{
namespace
{
namespace dl = kr::ps::dl;

dl::RuleProfile explicit_effects(dl::RuleProfile profile)
{
    profile.boolean_unconstrained_effects &= ~(profile.boolean_positive_effects | profile.boolean_negative_effects | profile.boolean_unchanged_effects);
    profile.numerical_unconstrained_effects &= ~(profile.numerical_increase_effects | profile.numerical_decrease_effects | profile.numerical_unchanged_effects);
    return profile;
}

dl::RuleProfile unchanged_boolean_loop(std::size_t num_booleans)
{
    auto rule = dl::RuleProfile(num_booleans, 0);
    const auto mask = num_booleans == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t { 1 } << num_booleans) - 1;
    rule.boolean_positive_conditions = mask;
    rule.boolean_unchanged_effects = mask;
    return explicit_effects(rule);
}

void expect_closed_cycle(const dl::CounterexampleComponent& component)
{
    const auto cycle = component.get_cycle();
    ASSERT_FALSE(cycle.empty());
    for (const auto position : cycle)
        ASSERT_LT(position, component.edges.size());
    for (std::size_t position = 0; position < cycle.size(); ++position)
        EXPECT_EQ(component.edges[cycle[position]].target_vertex_position, component.edges[cycle[(position + 1) % cycle.size()]].source_vertex_position);
}

}  // namespace

TEST(RunirTests, PublicStructuralTerminationReportsFailureFrontier)
{
    auto terminating = dl::QualitativePolicy(1, 0, 1);
    auto decrease = dl::RuleProfile(0, 1);
    decrease.numerical_greater_conditions |= 1;
    decrease.numerical_decrease_effects |= 1;
    terminating.rule_profiles.push_back(explicit_effects(std::move(decrease)));
    EXPECT_TRUE(dl::incomplete_structural_termination(terminating).get_cyclic_rule_positions().empty());
    for (const bool preprocess : { false, true })
    {
        const auto cleared = dl::structural_termination(terminating, dl::default_max_features, preprocess);
        EXPECT_TRUE(cleared.is_terminating());
        EXPECT_TRUE(cleared.get_cyclic_rule_positions().empty());
        EXPECT_TRUE(cleared.counterexample_components.empty());
    }

    auto policy = dl::QualitativePolicy(2, 1, 0);
    auto isolated_witness = dl::RuleProfile(1, 0, 0, 0);
    isolated_witness.boolean_negative_conditions |= 1;
    isolated_witness.boolean_positive_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(isolated_witness)));

    auto to_true = dl::RuleProfile(1, 0, 1, 1);
    to_true.boolean_negative_conditions |= 1;
    to_true.boolean_positive_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_true)));

    auto to_false = dl::RuleProfile(1, 0, 1, 1);
    to_false.boolean_positive_conditions |= 1;
    to_false.boolean_negative_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_false)));

    const auto result = dl::structural_termination(policy);

    EXPECT_EQ(result.status, dl::StructuralTerminationStatus::NON_TERMINATING);
    EXPECT_EQ(dl::incomplete_structural_termination(policy).get_cyclic_rule_positions(), std::vector<std::size_t>({ 1, 2 }));
    EXPECT_EQ(result.get_cyclic_rule_positions(), std::vector<std::size_t>({ 1, 2 }));
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
    expect_closed_cycle(counterexample);
    EXPECT_EQ(counterexample.get_cycle().size(), 2);
    EXPECT_EQ(counterexample.memory_positions, std::vector<std::size_t>({ 1 }));
    EXPECT_EQ(counterexample.boolean_positions, std::vector<std::size_t>({ 0 }));
    EXPECT_TRUE(counterexample.numerical_positions.empty());
    ASSERT_EQ(counterexample.vertices.size(), 2);
    ASSERT_EQ(counterexample.edges.size(), 2);
    for (const auto& vertex : counterexample.vertices)
        EXPECT_EQ(vertex.memory_position, 1);
    for (const auto& edge : counterexample.edges)
    {
        const auto source = (counterexample.vertices.at(edge.source_vertex_position).boolean_values & 1) != 0;
        const auto target = (counterexample.vertices.at(edge.target_vertex_position).boolean_values & 1) != 0;
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
    EXPECT_EQ(without_incomplete.get_cyclic_rule_positions(), result.get_cyclic_rule_positions());

    auto disjoint_cycles = dl::QualitativePolicy(1, 1, 0);
    auto false_loop = dl::RuleProfile(1, 0);
    false_loop.boolean_negative_conditions |= 1;
    false_loop.boolean_unchanged_effects |= 1;
    disjoint_cycles.rule_profiles.push_back(explicit_effects(std::move(false_loop)));
    auto true_loop = dl::RuleProfile(1, 0);
    true_loop.boolean_positive_conditions |= 1;
    true_loop.boolean_unchanged_effects |= 1;
    disjoint_cycles.rule_profiles.push_back(explicit_effects(std::move(true_loop)));

    const auto split = dl::structural_termination(disjoint_cycles);
    EXPECT_EQ(split.get_cyclic_rule_positions(), std::vector<std::size_t>({ 0, 1 }));
    ASSERT_EQ(split.counterexample_components.size(), 2);
    for (std::size_t position = 0; position < split.counterexample_components.size(); ++position)
    {
        const auto& component = split.counterexample_components[position];
        ASSERT_EQ(component.vertices.size(), 1);
        ASSERT_EQ(component.edges.size(), 1);
        EXPECT_EQ(component.get_cycle(), std::vector<std::size_t>({ 0 }));
        expect_closed_cycle(component);
        EXPECT_EQ(component.edges[0].rule_position, position);
        EXPECT_EQ(component.vertices[0].boolean_values, position);
    }
}

TEST(RunirTests, PublicHybridFrontierDoesNotRetainEliminationFalsePositives)
{
    auto policy = dl::QualitativePolicy(1, 2, 0);
    auto to_true = dl::RuleProfile(2, 0);
    to_true.boolean_negative_conditions |= 1;
    to_true.boolean_positive_conditions |= 2;
    to_true.boolean_positive_effects |= 1;
    to_true.boolean_unchanged_effects |= 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_true)));
    auto to_false = dl::RuleProfile(2, 0);
    to_false.boolean_positive_conditions |= 1;
    to_false.boolean_negative_conditions |= 2;
    to_false.boolean_negative_effects |= 1;
    to_false.boolean_unchanged_effects |= 2;
    policy.rule_profiles.push_back(explicit_effects(std::move(to_false)));

    // The unchanged selector separates the two apparent Boolean opponents.
    EXPECT_EQ(dl::incomplete_structural_termination(policy).get_cyclic_rule_positions(), std::vector<std::size_t>({ 0, 1 }));
    for (const bool preprocess : { false, true })
    {
        const auto result = dl::structural_termination(policy, dl::default_max_features, preprocess);
        EXPECT_TRUE(result.is_terminating());
        EXPECT_TRUE(result.get_cyclic_rule_positions().empty());
        EXPECT_TRUE(result.counterexample_components.empty());
    }
}

TEST(RunirTests, PublicCyclicRulesIncludeAllInstancesButExcludeAcyclicEdges)
{
    auto policy = dl::QualitativePolicy(1, 1, 0);
    auto shared = dl::RuleProfile(1, 0);
    shared.boolean_unchanged_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(shared)));
    auto false_loop = dl::RuleProfile(1, 0);
    false_loop.boolean_negative_conditions |= 1;
    false_loop.boolean_unchanged_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(false_loop)));
    policy.rule_profiles.push_back(unchanged_boolean_loop(1));
    auto set_true = dl::RuleProfile(1, 0);
    set_true.boolean_positive_effects |= 1;
    policy.rule_profiles.push_back(explicit_effects(std::move(set_true)));

    for (const bool preprocess : { false, true })
    {
        const auto result = dl::structural_termination(policy, dl::default_max_features, preprocess);
        EXPECT_EQ(result.get_cyclic_rule_positions(), std::vector<std::size_t>({ 0, 1, 2, 3 }));
        ASSERT_EQ(result.counterexample_components.size(), 2);
        std::size_t shared_instances = 0;
        std::size_t retained_assignments = 0;
        for (const auto& component : result.counterexample_components)
        {
            ASSERT_EQ(component.vertices.size(), 1);
            expect_closed_cycle(component);
            EXPECT_EQ(component.get_cycle().size(), 1);
            shared_instances += std::ranges::count(component.edges, 0, &dl::CounterexampleEdge::rule_position);
            retained_assignments += std::ranges::count(component.edges, 3, &dl::CounterexampleEdge::rule_position);
        }
        EXPECT_EQ(shared_instances, 2);
        EXPECT_EQ(retained_assignments, 1);  // false->true is acyclic; true->true remains.
    }
}

TEST(RunirTests, PublicFinalFrontierExcludesSurvivingCrossComponentRules)
{
    auto policy = dl::QualitativePolicy(2, 0, 0);
    policy.rule_profiles.emplace_back(0, 0, 0, 0);
    policy.rule_profiles.emplace_back(0, 0, 0, 1);
    policy.rule_profiles.emplace_back(0, 0, 1, 1);
    EXPECT_EQ(dl::incomplete_structural_termination(policy).get_cyclic_rule_positions(), std::vector<std::size_t>({ 0, 2 }));
    EXPECT_EQ(dl::incomplete_structural_termination(policy, false).get_cyclic_rule_positions(), std::vector<std::size_t>({ 0, 1, 2 }));
    for (const bool preprocess : { false, true })
    {
        const auto result = dl::structural_termination(policy, dl::default_max_features, preprocess, false);
        EXPECT_EQ(result.get_cyclic_rule_positions(), std::vector<std::size_t>({ 0, 2 }));
        ASSERT_EQ(result.counterexample_components.size(), 2);
        for (const auto& component : result.counterexample_components)
            expect_closed_cycle(component);
    }
}

TEST(RunirTests, PublicCyclePreservesOrderedEdgePositionsAndParallelRuleLabels)
{
    auto policy = dl::QualitativePolicy(2, 0, 0);
    policy.rule_profiles.emplace_back(0, 0, 1, 0);
    policy.rule_profiles.emplace_back(0, 0, 0, 1);
    policy.rule_profiles.emplace_back(0, 0, 0, 1);
    for (const bool preprocess : { false, true })
    {
        const auto result = dl::structural_termination(policy, dl::default_max_features, preprocess);
        EXPECT_EQ(result.get_cyclic_rule_positions(), std::vector<std::size_t>({ 0, 1, 2 }));
        ASSERT_EQ(result.counterexample_components.size(), 1);
        const auto& component = result.counterexample_components.front();
        ASSERT_EQ(component.edges.size(), 3);
        expect_closed_cycle(component);
        const auto cycle = component.get_cycle();
        ASSERT_EQ(cycle.size(), 2);
        auto rules = std::vector<std::size_t> {};
        for (const auto position : cycle)
            rules.push_back(component.edges.at(position).rule_position);
        std::ranges::sort(rules);
        EXPECT_EQ(rules.front(), 0);
        EXPECT_TRUE(rules.back() == 1 || rules.back() == 2);
    }
}

TEST(RunirTests, PublicStructuralTerminationPreserves64FeatureGlobalPositions)
{
    for (const auto& [num_booleans, num_numericals] : { std::pair<std::size_t, std::size_t> { 64, 0 }, { 0, 64 }, { 32, 32 } })
    {
        SCOPED_TRACE(num_booleans);
        auto policy = dl::QualitativePolicy(1, num_booleans, num_numericals);
        auto rule = dl::RuleProfile(num_booleans, num_numericals);
        if (num_booleans)
        {
            rule.boolean_positive_conditions |= std::uint64_t { 1 } << (num_booleans - 1);
            rule.boolean_unchanged_effects |= std::uint64_t { 1 } << (num_booleans - 1);
        }
        if (num_numericals)
        {
            rule.numerical_greater_conditions |= std::uint64_t { 1 } << (num_numericals - 1);
            rule.numerical_unchanged_effects |= std::uint64_t { 1 } << (num_numericals - 1);
        }
        policy.rule_profiles.push_back(explicit_effects(std::move(rule)));
        EXPECT_FALSE(dl::incomplete_structural_termination(policy).is_terminating());

        for (const auto preprocessing : { false, true })
        {
            SCOPED_TRACE(preprocessing);
            const auto result = dl::structural_termination(policy, dl::default_max_features, preprocessing);
            ASSERT_FALSE(result.is_terminating());
            ASSERT_EQ(result.counterexample_components.size(), 1);
            const auto& component = result.counterexample_components.front();
            EXPECT_EQ(component.boolean_positions.size(), num_booleans ? 1 : 0);
            EXPECT_EQ(component.numerical_positions.size(), num_numericals ? 1 : 0);
            ASSERT_EQ(component.vertices.size(), 1);
            EXPECT_EQ(component.vertices.front().boolean_values, num_booleans ? (std::uint64_t { 1 } << (num_booleans - 1)) : 0);
            EXPECT_EQ(component.vertices.front().numerical_values, num_numericals ? (std::uint64_t { 1 } << (num_numericals - 1)) : 0);
        }
    }
}

TEST(RunirTests, PublicStructuralTerminationRejects65FeaturesBeforePreprocessing)
{
    for (const auto& [num_booleans, num_numericals] : { std::pair<std::size_t, std::size_t> { 65, 0 }, { 0, 65 }, { 32, 33 } })
    {
        SCOPED_TRACE(num_booleans);
        EXPECT_THROW((void) dl::QualitativePolicy(1, num_booleans, num_numericals), std::invalid_argument);
        EXPECT_THROW((void) dl::RuleProfile(num_booleans, num_numericals), std::invalid_argument);
        auto policy = dl::QualitativePolicy(1, 0, 0);
        policy.num_booleans = num_booleans;
        policy.num_numericals = num_numericals;
        for (const auto with_terminating_rule : { false, true })
        {
            SCOPED_TRACE(with_terminating_rule);
            if (with_terminating_rule)
            {
                auto rule = dl::RuleProfile(0, 0);
                if (num_numericals)
                    rule.numerical_decrease_effects = 1;
                else
                {
                    rule.boolean_negative_conditions = 1;
                    rule.boolean_positive_effects = 1;
                }
                policy.rule_profiles.push_back(explicit_effects(std::move(rule)));
            }
            EXPECT_THROW((void) dl::incomplete_structural_termination(policy), std::invalid_argument);
            for (const auto preprocessing : { false, true })
                EXPECT_THROW((void) dl::structural_termination(policy, dl::default_max_features, preprocessing), std::invalid_argument);
        }
    }
    EXPECT_THROW((void) dl::QualitativePolicy(1, 1, std::numeric_limits<std::size_t>::max()), std::invalid_argument);
}

TEST(RunirTests, PublicStructuralTerminationValidatesEveryMaskBoundary)
{
    constexpr auto masks = std::array {
        std::pair { &dl::RuleProfile::boolean_positive_conditions, true },   std::pair { &dl::RuleProfile::boolean_negative_conditions, true },
        std::pair { &dl::RuleProfile::boolean_positive_effects, true },      std::pair { &dl::RuleProfile::boolean_negative_effects, true },
        std::pair { &dl::RuleProfile::boolean_unchanged_effects, true },     std::pair { &dl::RuleProfile::boolean_unconstrained_effects, true },
        std::pair { &dl::RuleProfile::numerical_greater_conditions, false }, std::pair { &dl::RuleProfile::numerical_zero_conditions, false },
        std::pair { &dl::RuleProfile::numerical_increase_effects, false },   std::pair { &dl::RuleProfile::numerical_decrease_effects, false },
        std::pair { &dl::RuleProfile::numerical_unchanged_effects, false },  std::pair { &dl::RuleProfile::numerical_unconstrained_effects, false },
    };
    for (std::size_t index = 0; index < masks.size(); ++index)
    {
        SCOPED_TRACE(index);
        const auto [mask, is_boolean] = masks[index];
        for (const auto num_features : { 0u, 1u, 63u, 64u })
        {
            SCOPED_TRACE(num_features);
            auto policy = dl::QualitativePolicy(1, is_boolean ? num_features : 0, is_boolean ? 0 : num_features);
            auto rule = dl::RuleProfile(is_boolean ? num_features : 0, is_boolean ? 0 : num_features);
            rule.*mask |= std::uint64_t { 1 } << (num_features == 64 ? 63 : num_features);
            policy.rule_profiles.push_back(explicit_effects(rule));
            if (num_features == 64)
                EXPECT_NO_THROW((void) dl::incomplete_structural_termination(policy));
            else
                EXPECT_THROW((void) dl::incomplete_structural_termination(policy), std::invalid_argument);
            for (const auto preprocessing : { false, true })
            {
                SCOPED_TRACE(preprocessing);
                if (num_features == 64)
                    EXPECT_NO_THROW((void) dl::structural_termination(policy, dl::default_max_features, preprocessing));
                else
                    EXPECT_THROW((void) dl::structural_termination(policy, dl::default_max_features, preprocessing), std::invalid_argument);
            }
        }
    }
}

TEST(RunirTests, PublicStructuralTerminationRejectsOverlappingEffects)
{
    constexpr auto effects = std::array {
        std::array { &dl::RuleProfile::boolean_positive_effects,
                     &dl::RuleProfile::boolean_negative_effects,
                     &dl::RuleProfile::boolean_unchanged_effects,
                     &dl::RuleProfile::boolean_unconstrained_effects },
        std::array { &dl::RuleProfile::numerical_increase_effects,
                     &dl::RuleProfile::numerical_decrease_effects,
                     &dl::RuleProfile::numerical_unchanged_effects,
                     &dl::RuleProfile::numerical_unconstrained_effects },
    };
    for (std::size_t kind = 0; kind < effects.size(); ++kind)
        for (std::size_t first = 0; first < effects[kind].size(); ++first)
            for (std::size_t second = first + 1; second < effects[kind].size(); ++second)
                for (const auto position : { 0u, 63u })
                {
                    SCOPED_TRACE(::testing::Message() << kind << ':' << first << ',' << second << " at " << position);
                    auto policy = dl::QualitativePolicy(1, kind == 0 ? position + 1 : 0, kind == 1 ? position + 1 : 0);
                    auto rule = dl::RuleProfile(policy.num_booleans, policy.num_numericals);
                    const auto bit = std::uint64_t { 1 } << position;
                    rule.*effects[kind][3] &= ~bit;
                    rule.*effects[kind][first] |= bit;
                    rule.*effects[kind][second] |= bit;
                    policy.rule_profiles.push_back(rule);
                    EXPECT_THROW((void) dl::incomplete_structural_termination(policy), std::invalid_argument);
                    for (const auto preprocessing : { false, true })
                        EXPECT_THROW((void) dl::structural_termination(policy, dl::default_max_features, preprocessing), std::invalid_argument);
                }
}

TEST(RunirTests, PublicStructuralTerminationRequiresCompleteEffectCoverage)
{
    for (const auto& [num_booleans, num_numericals] : { std::pair<std::size_t, std::size_t> { 0, 0 }, { 1, 0 }, { 0, 1 }, { 64, 0 }, { 0, 64 }, { 32, 32 } })
    {
        SCOPED_TRACE(::testing::Message() << num_booleans << ':' << num_numericals);
        auto policy = dl::QualitativePolicy(1, num_booleans, num_numericals);
        auto rule = dl::RuleProfile(num_booleans, num_numericals);
        const auto full = [](std::size_t size) { return size == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t { 1 } << size) - 1; };
        EXPECT_EQ((rule.effects<dl::BooleanFeature, dl::Unconstrained>()), full(num_booleans));
        EXPECT_EQ((rule.effects<dl::NumericalFeature, dl::Unconstrained>()), full(num_numericals));
        EXPECT_EQ(rule.boolean_positive_conditions | rule.boolean_negative_conditions | rule.numerical_greater_conditions | rule.numerical_zero_conditions, 0);
        EXPECT_EQ(rule.boolean_positive_effects | rule.boolean_negative_effects | rule.boolean_unchanged_effects, 0);
        EXPECT_EQ(rule.numerical_increase_effects | rule.numerical_decrease_effects | rule.numerical_unchanged_effects, 0);
        policy.rule_profiles.push_back(rule);
        EXPECT_NO_THROW((void) dl::incomplete_structural_termination(policy));
        for (const auto preprocessing : { false, true })
            EXPECT_NO_THROW((void) dl::structural_termination(policy, dl::default_max_features, preprocessing));

        for (const auto is_boolean : { false, true })
        {
            SCOPED_TRACE(is_boolean);
            const auto num_features = is_boolean ? num_booleans : num_numericals;
            if (num_features == 0)
                continue;
            policy.rule_profiles.back() = rule;
            auto& unconstrained =
                is_boolean ? policy.rule_profiles.back().boolean_unconstrained_effects : policy.rule_profiles.back().numerical_unconstrained_effects;
            unconstrained &= ~(std::uint64_t { 1 } << (num_features - 1));
            EXPECT_THROW((void) dl::incomplete_structural_termination(policy), std::invalid_argument);
            for (const auto preprocessing : { false, true })
                EXPECT_THROW((void) dl::structural_termination(policy, dl::default_max_features, preprocessing), std::invalid_argument);
        }
    }
}

TEST(RunirTests, PublicStructuralTerminationValidatesProfilesAndEnumeration)
{
    EXPECT_THROW((void) dl::QualitativePolicy(0, 0, 0), std::invalid_argument);

    auto bad_endpoint = dl::QualitativePolicy(1, 0, 0);
    bad_endpoint.rule_profiles.emplace_back(0, 0, 0, 1);
    EXPECT_THROW((void) dl::structural_termination(bad_endpoint), std::invalid_argument);
    EXPECT_THROW((void) dl::incomplete_structural_termination(bad_endpoint), std::invalid_argument);

    auto contradictory = dl::QualitativePolicy(1, 1, 0);
    contradictory.rule_profiles.emplace_back(1, 0);
    contradictory.rule_profiles.back().boolean_positive_conditions |= 1;
    contradictory.rule_profiles.back().boolean_negative_conditions |= 1;
    EXPECT_THROW((void) dl::structural_termination(contradictory), std::invalid_argument);
    EXPECT_THROW((void) dl::incomplete_structural_termination(contradictory), std::invalid_argument);

    auto contradictory_effect = dl::QualitativePolicy(1, 1, 0);
    contradictory_effect.rule_profiles.emplace_back(1, 0);
    contradictory_effect.rule_profiles.back().boolean_positive_effects |= 1;
    contradictory_effect.rule_profiles.back().boolean_unchanged_effects |= 1;
    contradictory_effect.rule_profiles.back() = explicit_effects(contradictory_effect.rule_profiles.back());
    EXPECT_THROW((void) dl::structural_termination(contradictory_effect), std::invalid_argument);
    EXPECT_THROW((void) dl::incomplete_structural_termination(contradictory_effect), std::invalid_argument);

    auto contradictory_numerical = dl::QualitativePolicy(1, 0, 1);
    contradictory_numerical.rule_profiles.emplace_back(0, 1);
    contradictory_numerical.rule_profiles.back().numerical_greater_conditions |= 1;
    contradictory_numerical.rule_profiles.back().numerical_zero_conditions |= 1;
    EXPECT_THROW((void) dl::structural_termination(contradictory_numerical), std::invalid_argument);
    EXPECT_THROW((void) dl::incomplete_structural_termination(contradictory_numerical), std::invalid_argument);

    auto impossible_transition = dl::QualitativePolicy(1, 0, 1);
    impossible_transition.rule_profiles.emplace_back(0, 1);
    impossible_transition.rule_profiles.back().numerical_zero_conditions |= 1;
    impossible_transition.rule_profiles.back().numerical_decrease_effects = 1;
    impossible_transition.rule_profiles.back() = explicit_effects(impossible_transition.rule_profiles.back());
    EXPECT_TRUE(dl::structural_termination(impossible_transition, dl::default_max_features, false).is_terminating());

    auto over_limit = dl::QualitativePolicy(1, 1, 0);
    over_limit.rule_profiles.push_back(unchanged_boolean_loop(1));
    EXPECT_THROW((void) dl::structural_termination(over_limit, 0, false), std::invalid_argument);
    EXPECT_THROW((void) dl::structural_termination(over_limit, 0), std::invalid_argument);

    constexpr auto digits = std::numeric_limits<std::size_t>::digits;
    auto shift_overflow = dl::QualitativePolicy(1, digits, 0);
    shift_overflow.rule_profiles.push_back(unchanged_boolean_loop(digits));
    EXPECT_THROW((void) dl::structural_termination(shift_overflow, digits, false), std::invalid_argument);

    auto multiplication_overflow = dl::QualitativePolicy(2, digits - 1, 0);
    auto forward = unchanged_boolean_loop(digits - 1);
    forward.target_memory_position = 1;
    multiplication_overflow.rule_profiles.push_back(explicit_effects(std::move(forward)));
    auto backward = unchanged_boolean_loop(digits - 1);
    backward.source_memory_position = 1;
    multiplication_overflow.rule_profiles.push_back(explicit_effects(std::move(backward)));
    EXPECT_THROW((void) dl::structural_termination(multiplication_overflow, digits - 1, false), std::invalid_argument);
}

}  // namespace runir::tests
