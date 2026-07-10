#include "detail.hpp"

namespace runir::kr::ps::base::dl::detail
{

namespace
{

IncompleteRuleChanges make_incomplete_changes(const RuleProfile& profile)
{
    auto changes = IncompleteRuleChanges {};
    changes.boolean_to_true = profile.boolean_negative_conditions & profile.boolean_positive_effects;
    changes.boolean_to_false = profile.boolean_positive_conditions & profile.boolean_negative_effects;
    changes.numerical_decreases.resize(profile.numerical_changes.size());
    changes.numerical_increases_or_unconstrained.resize(profile.numerical_changes.size());
    for (std::size_t position = 0; position < profile.numerical_changes.size(); ++position)
    {
        if (profile.numerical_changes[position] == NumericalChange::DECREASES)
            changes.numerical_decreases.set(position);
        if (profile.numerical_changes[position] == NumericalChange::INCREASES || profile.numerical_changes[position] == NumericalChange::UNCONSTRAINED)
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

}  // namespace

std::vector<std::size_t> opposing_rules(const SketchAnalysis& analysis,
                                        const IncompleteSieveResult& sieve_result,
                                        std::size_t rule_position,
                                        bool is_boolean,
                                        std::size_t feature_position,
                                        bool to_true)
{
    auto opposing = std::vector<std::size_t> {};
    for (std::size_t other = 0; other < analysis.rules.size(); ++other)
    {
        if (other == rule_position || !sieve_result.remaining[other])
            continue;

        const auto opposes = is_boolean ? (to_true ? sieve_result.changes[other].boolean_to_false.test(feature_position) :
                                                     sieve_result.changes[other].boolean_to_true.test(feature_position)) :
                                          sieve_result.changes[other].numerical_increases_or_unconstrained.test(feature_position);
        if (opposes && !r3_discounts(analysis.profiles[rule_position], analysis.profiles[other], sieve_result.marked_booleans, sieve_result.marked_numericals))
            opposing.push_back(other);
    }
    return opposing;
}

IncompleteSieveResult run_incomplete_sieve(const SketchAnalysis& analysis)
{
    auto result = IncompleteSieveResult {};
    result.remaining = std::vector<bool>(analysis.rules.size(), true);
    result.marked_booleans.resize(analysis.features.booleans.size());
    result.marked_numericals.resize(analysis.features.numericals.size());
    for (const auto& profile : analysis.profiles)
        result.changes.push_back(make_incomplete_changes(profile));

    auto eliminated = true;
    while (eliminated)
    {
        eliminated = false;
        for (std::size_t rule_position = 0; rule_position < analysis.rules.size() && !eliminated; ++rule_position)
        {
            if (!result.remaining[rule_position])
                continue;

            for (std::size_t position = 0; position < analysis.features.numericals.size() && !eliminated; ++position)
            {
                if (!result.changes[rule_position].numerical_decreases.test(position))
                    continue;
                if (opposing_rules(analysis, result, rule_position, false, position, false).empty())
                {
                    result.remaining[rule_position] = false;
                    result.marked_numericals.set(position);
                    eliminated = true;
                }
            }
            for (std::size_t position = 0; position < analysis.features.booleans.size() && !eliminated; ++position)
            {
                const auto to_true = result.changes[rule_position].boolean_to_true.test(position);
                const auto to_false = result.changes[rule_position].boolean_to_false.test(position);
                if (!to_true && !to_false)
                    continue;
                if (opposing_rules(analysis, result, rule_position, true, position, to_true).empty())
                {
                    result.remaining[rule_position] = false;
                    result.marked_booleans.set(position);
                    eliminated = true;
                }
            }
        }
    }
    return result;
}

}  // namespace runir::kr::ps::base::dl::detail
