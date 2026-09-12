#include "detail.hpp"
#include "scc_refinement_forest.hpp"

#include <algorithm>

namespace runir::kr::ps::detail
{

namespace
{

struct RuleChanges
{
    std::uint64_t boolean_to_true = 0;
    std::uint64_t boolean_to_false = 0;
    std::uint64_t boolean_may_become_true = 0;
    std::uint64_t boolean_may_become_false = 0;
};

struct SieveState
{
    std::vector<RuleChanges> changes;
    std::vector<bool> remaining;
    ResidualMemorySccs memory_sccs;

    SieveState(const QualitativePolicy& policy, bool use_memory_scc_scope) :
        remaining(policy.rule_profiles.size(), true),
        memory_sccs(policy, use_memory_scc_scope)
    {
    }
};

struct OpposingRuleSet
{
    bool has_raw_opponent = false;
    std::vector<std::size_t> undiscounted;
};

RuleChanges make_changes(const RuleProfile& profile)
{
    auto changes = RuleChanges {};
    changes.boolean_to_true = profile.boolean_negative_conditions & profile.boolean_positive_effects;
    changes.boolean_to_false = profile.boolean_positive_conditions & profile.boolean_negative_effects;

    changes.boolean_may_become_true = (profile.boolean_positive_effects | profile.boolean_unconstrained_effects) & ~profile.boolean_positive_conditions;
    changes.boolean_may_become_false = (profile.boolean_negative_effects | profile.boolean_unconstrained_effects) & ~profile.boolean_negative_conditions;

    return changes;
}

bool r3_discounts(const RuleProfile& rule, const RuleProfile& opposing, const SccRefinementForest::Marks& marks)
{
    return (rule.boolean_positive_conditions & opposing.boolean_negative_conditions & marks.booleans)
           || (rule.boolean_negative_conditions & opposing.boolean_positive_conditions & marks.booleans)
           || (rule.numerical_greater_conditions & opposing.numerical_zero_conditions & marks.numericals)
           || (rule.numerical_zero_conditions & opposing.numerical_greater_conditions & marks.numericals);
}

std::vector<std::size_t> remaining_rule_positions(const SieveState& state)
{
    auto result = std::vector<std::size_t> {};
    result.reserve(state.remaining.size());
    for (std::size_t rule_position = 0; rule_position < state.remaining.size(); ++rule_position)
        if (state.remaining[rule_position])
            result.push_back(rule_position);
    return result;
}

bool remove_acyclic_rules(const QualitativePolicy& policy, SieveState& state)
{
    auto removed = false;
    for (std::size_t rule_position = 0; rule_position < policy.rule_profiles.size(); ++rule_position)
    {
        if (!state.remaining[rule_position])
            continue;
        if (!state.memory_sccs.is_cross_scc_rule(rule_position))
            continue;
        state.remaining[rule_position] = false;
        removed = true;
    }
    return removed;
}

OpposingRuleSet opposing_rules(const QualitativePolicy& policy,
                               const SieveState& state,
                               std::size_t rule_position,
                               IncompletePolicyResult::FeatureKind feature_kind,
                               std::size_t feature_position,
                               bool towards_positive = false)
{
    auto opposing = OpposingRuleSet {};
    const auto feature = std::uint64_t { 1 } << feature_position;
    for (std::size_t other = 0; other < policy.rule_profiles.size(); ++other)
    {
        if (other == rule_position || !state.remaining[other] || !state.memory_sccs.share_opponent_scope(rule_position, other))
            continue;

        const auto& profile = policy.rule_profiles[other];
        const auto opposes =
            feature_kind == IncompletePolicyResult::FeatureKind::BOOLEAN ?
                bool(feature & (towards_positive ? state.changes[other].boolean_may_become_false : state.changes[other].boolean_may_become_true)) :
                bool(
                    feature
                    & ((towards_positive ? profile.numerical_decrease_effects : profile.numerical_increase_effects) | profile.numerical_unconstrained_effects));
        if (!opposes)
            continue;

        opposing.has_raw_opponent = true;
        const auto marks = state.memory_sccs.marks_for(rule_position);
        if (!r3_discounts(policy.rule_profiles[rule_position], policy.rule_profiles[other], marks))
            opposing.undiscounted.push_back(other);
    }
    return opposing;
}

SieveState make_state(const QualitativePolicy& policy, bool use_memory_scc_scope)
{
    auto state = SieveState(policy, use_memory_scc_scope);
    state.changes.reserve(policy.rule_profiles.size());
    for (const auto& profile : policy.rule_profiles)
        state.changes.push_back(make_changes(profile));
    return state;
}

enum class EliminationPass
{
    MARKING,
    R3
};

bool eligible(const OpposingRuleSet& opposing, EliminationPass pass)
{
    return pass == EliminationPass::MARKING ? !opposing.has_raw_opponent : opposing.has_raw_opponent && opposing.undiscounted.empty();
}

bool eliminate_one_rule(const QualitativePolicy& policy, SieveState& state, EliminationPass pass)
{
    for (std::size_t rule_position = 0; rule_position < policy.rule_profiles.size(); ++rule_position)
    {
        if (!state.remaining[rule_position])
            continue;

        for (std::size_t position = 0; position < policy.num_numericals; ++position)
        {
            const auto feature = std::uint64_t { 1 } << position;
            const bool decreases = policy.rule_profiles[rule_position].numerical_decrease_effects & feature;
            const bool increases = policy.rule_profiles[rule_position].numerical_increase_effects & feature;
            if (!decreases && !increases)
                continue;
            const auto opposing = opposing_rules(policy, state, rule_position, IncompletePolicyResult::FeatureKind::NUMERICAL, position, increases);
            if (eligible(opposing, pass))
            {
                state.remaining[rule_position] = false;
                if (pass == EliminationPass::MARKING)
                    state.memory_sccs.establish_r1_mark(rule_position, position);
                return true;
            }
        }
        for (std::size_t position = 0; position < policy.num_booleans; ++position)
        {
            const auto feature = std::uint64_t { 1 } << position;
            const bool to_true = state.changes[rule_position].boolean_to_true & feature;
            const bool to_false = state.changes[rule_position].boolean_to_false & feature;
            if (!to_true && !to_false)
                continue;
            const auto opposing = opposing_rules(policy, state, rule_position, IncompletePolicyResult::FeatureKind::BOOLEAN, position, to_true);
            if (eligible(opposing, pass))
            {
                state.remaining[rule_position] = false;
                if (pass == EliminationPass::MARKING)
                    state.memory_sccs.establish_r2_mark(rule_position, position);
                return true;
            }
        }
    }
    return false;
}

SieveState run_sieve(const QualitativePolicy& policy, bool use_memory_scc_scope)
{
    auto state = make_state(policy, use_memory_scc_scope);
    while (true)
    {
        if (eliminate_one_rule(policy, state, EliminationPass::MARKING))
            continue;
        if (eliminate_one_rule(policy, state, EliminationPass::R3))
            continue;
        const auto refined = state.memory_sccs.refine(remaining_rule_positions(state));
        const auto removed_acyclic = remove_acyclic_rules(policy, state);
        if (!refined && !removed_acyclic)
            return state;
    }
}

std::vector<IncompletePolicyResult::ResidualMemoryScope> make_residual_scopes(const SieveState& state)
{
    const auto& forest = state.memory_sccs.refinement_forest();
    auto result = std::vector<IncompletePolicyResult::ResidualMemoryScope> {};
    for (SccRefinementForest::NodeIndex node_position = 0; node_position < forest.nodes().size(); ++node_position)
    {
        const auto& node = forest.node(node_position);
        if (!node.is_leaf())
            continue;

        auto scope = IncompletePolicyResult::ResidualMemoryScope { node.memory_positions, {} };
        const auto marks = forest.effective_marks(node_position);
        for (std::size_t feature_position = 0; feature_position < marks.boolean_witnessing_rule_positions.size(); ++feature_position)
            if (marks.booleans & (std::uint64_t { 1 } << feature_position))
                scope.marked_features.push_back(IncompletePolicyResult::MarkedFeature { IncompletePolicyResult::FeatureKind::BOOLEAN,
                                                                                        feature_position,
                                                                                        marks.boolean_witnessing_rule_positions[feature_position] });
        for (std::size_t feature_position = 0; feature_position < marks.numerical_witnessing_rule_positions.size(); ++feature_position)
            if (marks.numericals & (std::uint64_t { 1 } << feature_position))
                scope.marked_features.push_back(IncompletePolicyResult::MarkedFeature { IncompletePolicyResult::FeatureKind::NUMERICAL,
                                                                                        feature_position,
                                                                                        marks.numerical_witnessing_rule_positions[feature_position] });
        result.push_back(std::move(scope));
    }
    std::ranges::sort(result, {}, [](const auto& scope) -> const auto& { return scope.memory_positions; });
    return result;
}

}  // namespace

IncompletePolicyResult incomplete_structural_termination(const QualitativePolicy& policy, bool use_memory_scc_scope)
{
    const auto state = run_sieve(policy, use_memory_scc_scope);
    auto result = IncompletePolicyResult { make_residual_scopes(state), {} };
    for (std::size_t rule_position = 0; rule_position < policy.rule_profiles.size(); ++rule_position)
    {
        if (!state.remaining[rule_position])
            continue;

        auto surviving = IncompletePolicyResult::SurvivingRule { rule_position, {} };
        for (std::size_t position = 0; position < policy.num_numericals; ++position)
        {
            const auto feature = std::uint64_t { 1 } << position;
            const bool decreases = policy.rule_profiles[rule_position].numerical_decrease_effects & feature;
            const bool increases = policy.rule_profiles[rule_position].numerical_increase_effects & feature;
            if (!decreases && !increases)
                continue;
            surviving.blocking_reasons.push_back(IncompletePolicyResult::BlockingReason {
                IncompletePolicyResult::FeatureKind::NUMERICAL,
                position,
                opposing_rules(policy, state, rule_position, IncompletePolicyResult::FeatureKind::NUMERICAL, position, increases).undiscounted,
            });
        }
        for (std::size_t position = 0; position < policy.num_booleans; ++position)
        {
            const auto feature = std::uint64_t { 1 } << position;
            const bool to_true = state.changes[rule_position].boolean_to_true & feature;
            const bool to_false = state.changes[rule_position].boolean_to_false & feature;
            if (!to_true && !to_false)
                continue;
            surviving.blocking_reasons.push_back(IncompletePolicyResult::BlockingReason {
                IncompletePolicyResult::FeatureKind::BOOLEAN,
                position,
                opposing_rules(policy, state, rule_position, IncompletePolicyResult::FeatureKind::BOOLEAN, position, to_true).undiscounted,
            });
        }
        result.surviving_rules.push_back(std::move(surviving));
    }
    return result;
}

}  // namespace runir::kr::ps::detail
