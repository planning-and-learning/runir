#include "detail.hpp"

namespace runir::kr::ps::detail
{

namespace
{

struct RuleChanges
{
    boost::dynamic_bitset<> boolean_to_true;
    boost::dynamic_bitset<> boolean_to_false;
    boost::dynamic_bitset<> boolean_may_become_true;
    boost::dynamic_bitset<> boolean_may_become_false;
    boost::dynamic_bitset<> numerical_decreases;
    boost::dynamic_bitset<> numerical_increases_or_unconstrained;
};

struct SieveState
{
    std::vector<RuleChanges> changes;
    std::vector<bool> remaining;
    boost::dynamic_bitset<> marked_booleans;
    boost::dynamic_bitset<> marked_numericals;
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

    auto unconstrained = boost::dynamic_bitset<>(profile.boolean_positive_effects.size());
    unconstrained.set();
    const auto specified = profile.boolean_positive_effects | profile.boolean_negative_effects | profile.boolean_unchanged_effects;
    unconstrained = unconstrained - specified;
    changes.boolean_may_become_true = (profile.boolean_positive_effects | unconstrained) - profile.boolean_positive_conditions;
    changes.boolean_may_become_false = (profile.boolean_negative_effects | unconstrained) - profile.boolean_negative_conditions;

    changes.numerical_decreases.resize(profile.numerical_changes.size());
    changes.numerical_increases_or_unconstrained.resize(profile.numerical_changes.size());
    for (std::size_t position = 0; position < profile.numerical_changes.size(); ++position)
    {
        if (profile.numerical_changes[position] == dl::NumericalChange::DECREASES)
            changes.numerical_decreases.set(position);
        if (profile.numerical_changes[position] == dl::NumericalChange::INCREASES || profile.numerical_changes[position] == dl::NumericalChange::UNCONSTRAINED)
            changes.numerical_increases_or_unconstrained.set(position);
    }
    return changes;
}

bool r3_discounts(const RuleProfile& rule,
                  const RuleProfile& opposing,
                  const boost::dynamic_bitset<>& marked_booleans,
                  const boost::dynamic_bitset<>& marked_numericals)
{
    return (rule.boolean_positive_conditions & opposing.boolean_negative_conditions & marked_booleans).any()
           || (rule.boolean_negative_conditions & opposing.boolean_positive_conditions & marked_booleans).any()
           || (rule.numerical_greater_conditions & opposing.numerical_zero_conditions & marked_numericals).any()
           || (rule.numerical_zero_conditions & opposing.numerical_greater_conditions & marked_numericals).any();
}

OpposingRuleSet opposing_rules(const QualitativePolicy& policy,
                               const SieveState& state,
                               std::size_t rule_position,
                               IncompletePolicyResult::FeatureKind feature_kind,
                               std::size_t feature_position,
                               bool to_true = false)
{
    auto opposing = OpposingRuleSet {};
    for (std::size_t other = 0; other < policy.rule_profiles.size(); ++other)
    {
        if (other == rule_position || !state.remaining[other])
            continue;

        const auto opposes = feature_kind == IncompletePolicyResult::FeatureKind::BOOLEAN ?
                                 (to_true ? state.changes[other].boolean_may_become_false.test(feature_position) :
                                            state.changes[other].boolean_may_become_true.test(feature_position)) :
                                 state.changes[other].numerical_increases_or_unconstrained.test(feature_position);
        if (!opposes)
            continue;

        opposing.has_raw_opponent = true;
        if (!r3_discounts(policy.rule_profiles[rule_position], policy.rule_profiles[other], state.marked_booleans, state.marked_numericals))
            opposing.undiscounted.push_back(other);
    }
    return opposing;
}

SieveState run_sieve(const QualitativePolicy& policy)
{
    auto state = SieveState {
        {},
        std::vector<bool>(policy.rule_profiles.size(), true),
        boost::dynamic_bitset<>(policy.num_booleans),
        boost::dynamic_bitset<>(policy.num_numericals),
    };
    state.changes.reserve(policy.rule_profiles.size());
    for (const auto& profile : policy.rule_profiles)
        state.changes.push_back(make_changes(profile));

    auto eliminated = true;
    while (eliminated)
    {
        eliminated = false;
        for (std::size_t rule_position = 0; rule_position < policy.rule_profiles.size() && !eliminated; ++rule_position)
        {
            if (!state.remaining[rule_position])
                continue;

            for (std::size_t position = 0; position < policy.num_numericals && !eliminated; ++position)
            {
                if (!state.changes[rule_position].numerical_decreases.test(position))
                    continue;
                const auto opposing = opposing_rules(policy, state, rule_position, IncompletePolicyResult::FeatureKind::NUMERICAL, position);
                if (opposing.undiscounted.empty())
                {
                    state.remaining[rule_position] = false;
                    // R3 consumes existing stabilization marks but cannot establish a new one.
                    if (!opposing.has_raw_opponent)
                        state.marked_numericals.set(position);
                    eliminated = true;
                }
            }
            for (std::size_t position = 0; position < policy.num_booleans && !eliminated; ++position)
            {
                const auto to_true = state.changes[rule_position].boolean_to_true.test(position);
                const auto to_false = state.changes[rule_position].boolean_to_false.test(position);
                if (!to_true && !to_false)
                    continue;
                const auto opposing = opposing_rules(policy, state, rule_position, IncompletePolicyResult::FeatureKind::BOOLEAN, position, to_true);
                if (opposing.undiscounted.empty())
                {
                    state.remaining[rule_position] = false;
                    if (!opposing.has_raw_opponent)
                        state.marked_booleans.set(position);
                    eliminated = true;
                }
            }
        }
    }
    return state;
}

}  // namespace

IncompletePolicyResult incomplete_structural_termination(const QualitativePolicy& policy)
{
    const auto state = run_sieve(policy);
    auto result = IncompletePolicyResult {};
    for (std::size_t rule_position = 0; rule_position < policy.rule_profiles.size(); ++rule_position)
    {
        if (!state.remaining[rule_position])
            continue;

        auto surviving = IncompletePolicyResult::SurvivingRule { rule_position, {} };
        for (std::size_t position = 0; position < policy.num_numericals; ++position)
        {
            if (!state.changes[rule_position].numerical_decreases.test(position))
                continue;
            surviving.blocking_reasons.push_back(IncompletePolicyResult::BlockingReason {
                IncompletePolicyResult::FeatureKind::NUMERICAL,
                position,
                opposing_rules(policy, state, rule_position, IncompletePolicyResult::FeatureKind::NUMERICAL, position).undiscounted,
            });
        }
        for (std::size_t position = 0; position < policy.num_booleans; ++position)
        {
            const auto to_true = state.changes[rule_position].boolean_to_true.test(position);
            const auto to_false = state.changes[rule_position].boolean_to_false.test(position);
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
