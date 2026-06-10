#ifndef RUNIR_KR_PS_BASE_DL_INCOMPLETE_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_BASE_DL_INCOMPLETE_STRUCTURAL_TERMINATION_HPP_

#include "runir/config.hpp"
#include "runir/kr/ps/base/dl/structural_termination.hpp"

#include <cstdint>
#include <optional>
#include <vector>

#if RUNIR_ENABLE_FMT_FORMATTERS
#include <fmt/format.h>
#include <string>
#include <string_view>
#endif

namespace runir::kr::ps::base::dl
{

/**
 * Incomplete syntactic structural termination test.
 *
 * Implements the rule-elimination procedure of incomplete_sieve.pdf
 * Section 5.1 (Theorem 4: sound, not complete). The procedure iteratively
 * applies one of the following cases until no rule is left (terminating) or
 * no case applies (unknown):
 *
 *  R1: eliminate a rule r that decreases a numerical feature n that no other
 *      remaining rule can increase (an explicit increase or an unconstrained
 *      "n?" effect, encoded by omission in the runir sketch language), and
 *      mark n;
 *  R2: eliminate a rule r that changes a Boolean feature p (p in C and
 *      "not p" in E, or vice versa) that no other remaining rule changes in
 *      the opposite direction, and mark p;
 *  R3: as R1/R2, except that an opposing rule r' is discounted when the
 *      conditions C of r contain a condition on a marked feature that is
 *      complementary to a condition in C' of r' (e.g. m > 0 in C and m = 0
 *      in C', or q in C and "not q" in C').
 *
 * A "terminating" verdict is sound; "unknown" is not a proof of
 * non-termination (use structural_termination() for the complete test).
 *
 * On an unknown verdict, the result lists the surviving rules, each
 * annotated with the reasons that block its elimination: the feature whose
 * decrease/flip the rule performs and the remaining opposing rules that
 * R1/R2 see and that R3 cannot discount.
 */

enum class IncompleteStructuralTerminationStatus
{
    TERMINATING,
    UNKNOWN,
};

/// Why one elimination attempt for a surviving rule is blocked.
struct IncompleteBlockingReason
{
    /// Position into the result's boolean (numerical) feature list; exactly
    /// one of the two is set.
    std::optional<std::size_t> boolean_position;
    std::optional<std::size_t> numerical_position;
    /// The remaining rules that oppose the change and survive R3's
    /// marked-complementary-condition check.
    std::vector<RuleView> opposing_rules;
};

struct IncompleteSurvivingRule
{
    RuleView rule;
    /// One entry per feature the rule decreases (numerical) or flips
    /// (Boolean); empty iff the rule performs no eliminating change at all.
    std::vector<IncompleteBlockingReason> blocking_reasons;
};

struct IncompleteStructuralTerminationResult
{
    IncompleteStructuralTerminationStatus status = IncompleteStructuralTerminationStatus::TERMINATING;
    std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>> booleans;
    std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>> numericals;
    std::vector<IncompleteSurvivingRule> surviving_rules;  ///< empty iff terminating.

    bool is_terminating() const noexcept { return status == IncompleteStructuralTerminationStatus::TERMINATING; }
};

namespace detail
{

/// Boolean changes of a rule per incomplete_sieve.pdf Section 5.1: a rule
/// changes p iff p is in the conditions and "not p" in the effects or vice
/// versa.
struct IncompleteRuleChanges
{
    std::uint32_t boolean_to_true = 0;   // "not p" in C and p in E
    std::uint32_t boolean_to_false = 0;  // p in C and "not p" in E
    std::uint32_t numerical_decreases = 0;
    std::uint32_t numerical_increases_or_unconstrained = 0;
};

inline IncompleteRuleChanges make_incomplete_changes(const RuleProfile& profile, std::size_t num_numericals)
{
    auto changes = IncompleteRuleChanges {};
    changes.boolean_to_true = profile.boolean_negative_conditions & profile.boolean_positive_effects;
    changes.boolean_to_false = profile.boolean_positive_conditions & profile.boolean_negative_effects;
    for (std::size_t position = 0; position < num_numericals; ++position)
    {
        const auto change = static_cast<NumericalChange>((profile.numerical_changes >> (2 * position)) & std::uint64_t { 3 });
        if (change == NumericalChange::DECREASES)
            changes.numerical_decreases |= std::uint32_t { 1 } << position;
        if (change == NumericalChange::INCREASES || change == NumericalChange::UNCONSTRAINED)
            changes.numerical_increases_or_unconstrained |= std::uint32_t { 1 } << position;
    }
    return changes;
}

/// R3: r' is discounted when C(r) constrains a marked feature in a way
/// complementary to C(r').
inline bool r3_discounts(const RuleProfile& rule,
                         const RuleProfile& opposing,
                         std::uint32_t marked_booleans,
                         std::uint32_t marked_numericals)
{
    if ((rule.boolean_positive_conditions & opposing.boolean_negative_conditions & marked_booleans) != 0)
        return true;
    if ((rule.boolean_negative_conditions & opposing.boolean_positive_conditions & marked_booleans) != 0)
        return true;
    if ((rule.numerical_greater_conditions & opposing.numerical_zero_conditions & marked_numericals) != 0)
        return true;
    if ((rule.numerical_zero_conditions & opposing.numerical_greater_conditions & marked_numericals) != 0)
        return true;
    return false;
}

}  // namespace detail

inline IncompleteStructuralTerminationResult incomplete_structural_termination(SketchView sketch)
{
    auto features = FeatureSyntacticComplexityContext {};
    collect_features(features, sketch);

    const auto num_booleans = features.booleans.size();
    const auto num_numericals = features.numericals.size();
    if (num_booleans > 32 || num_numericals > 32)
        throw std::invalid_argument("incomplete_structural_termination: more than 32 features of one kind are not supported.");

    auto rules = std::vector<RuleView> {};
    for (auto rule : sketch.get_rules())
        rules.push_back(rule);

    auto profiles = std::vector<detail::RuleProfile> {};
    auto changes = std::vector<detail::IncompleteRuleChanges> {};
    for (auto rule : rules)
    {
        profiles.push_back(detail::make_rule_profile(features, rule));
        changes.push_back(detail::make_incomplete_changes(profiles.back(), num_numericals));
    }

    auto remaining = std::vector<bool>(rules.size(), true);
    auto marked_booleans = std::uint32_t { 0 };
    auto marked_numericals = std::uint32_t { 0 };

    // Collect, for rule r and one changed feature, the remaining opposing
    // rules that R3 cannot discount.
    const auto opposing_rules = [&](std::size_t rule_position, bool is_boolean, std::size_t feature_position, bool to_true)
    {
        const auto bit = std::uint32_t { 1 } << feature_position;
        auto opposing = std::vector<std::size_t> {};
        for (std::size_t other = 0; other < rules.size(); ++other)
        {
            if (other == rule_position || !remaining[other])
                continue;
            auto opposes = false;
            if (is_boolean)
            {
                // R2: only explicit opposite changes oppose.
                opposes = to_true ? (changes[other].boolean_to_false & bit) != 0 : (changes[other].boolean_to_true & bit) != 0;
            }
            else
            {
                // R1: an explicit increase or an unconstrained effect opposes.
                opposes = (changes[other].numerical_increases_or_unconstrained & bit) != 0;
            }
            if (opposes && !detail::r3_discounts(profiles[rule_position], profiles[other], marked_booleans, marked_numericals))
                opposing.push_back(other);
        }
        return opposing;
    };

    auto eliminated = true;
    while (eliminated)
    {
        eliminated = false;
        for (std::size_t rule_position = 0; rule_position < rules.size() && !eliminated; ++rule_position)
        {
            if (!remaining[rule_position])
                continue;

            for (std::size_t position = 0; position < num_numericals && !eliminated; ++position)
            {
                if ((changes[rule_position].numerical_decreases & (std::uint32_t { 1 } << position)) == 0)
                    continue;
                if (opposing_rules(rule_position, false, position, false).empty())
                {
                    remaining[rule_position] = false;
                    marked_numericals |= std::uint32_t { 1 } << position;
                    eliminated = true;
                }
            }
            for (std::size_t position = 0; position < num_booleans && !eliminated; ++position)
            {
                const auto bit = std::uint32_t { 1 } << position;
                const auto to_true = (changes[rule_position].boolean_to_true & bit) != 0;
                const auto to_false = (changes[rule_position].boolean_to_false & bit) != 0;
                if (!to_true && !to_false)
                    continue;
                if (opposing_rules(rule_position, true, position, to_true).empty())
                {
                    remaining[rule_position] = false;
                    marked_booleans |= bit;
                    eliminated = true;
                }
            }
        }
    }

    auto result = IncompleteStructuralTerminationResult {};
    result.booleans = features.booleans;
    result.numericals = features.numericals;

    for (std::size_t rule_position = 0; rule_position < rules.size(); ++rule_position)
    {
        if (!remaining[rule_position])
            continue;

        auto surviving = IncompleteSurvivingRule { rules[rule_position], {} };
        for (std::size_t position = 0; position < num_numericals; ++position)
        {
            if ((changes[rule_position].numerical_decreases & (std::uint32_t { 1 } << position)) == 0)
                continue;
            auto reason = IncompleteBlockingReason {};
            reason.numerical_position = position;
            for (auto other : opposing_rules(rule_position, false, position, false))
                reason.opposing_rules.push_back(rules[other]);
            surviving.blocking_reasons.push_back(std::move(reason));
        }
        for (std::size_t position = 0; position < num_booleans; ++position)
        {
            const auto bit = std::uint32_t { 1 } << position;
            const auto to_true = (changes[rule_position].boolean_to_true & bit) != 0;
            const auto to_false = (changes[rule_position].boolean_to_false & bit) != 0;
            if (!to_true && !to_false)
                continue;
            auto reason = IncompleteBlockingReason {};
            reason.boolean_position = position;
            for (auto other : opposing_rules(rule_position, true, position, to_true))
                reason.opposing_rules.push_back(rules[other]);
            surviving.blocking_reasons.push_back(std::move(reason));
        }
        result.surviving_rules.push_back(std::move(surviving));
    }

    result.status = result.surviving_rules.empty() ? IncompleteStructuralTerminationStatus::TERMINATING :
                                                     IncompleteStructuralTerminationStatus::UNKNOWN;
    return result;
}

}  // namespace runir::kr::ps::base::dl

#if RUNIR_ENABLE_FMT_FORMATTERS
namespace fmt
{

template<>
struct formatter<runir::kr::ps::base::dl::IncompleteStructuralTerminationResult, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::base::dl::IncompleteStructuralTerminationResult& result, FormatContext& ctx) const
    {
        auto text = std::string {};
        if (result.is_terminating())
        {
            text = "IncompleteStructuralTerminationResult(terminating)";
        }
        else
        {
            text = fmt::format("IncompleteStructuralTerminationResult(unknown, {} surviving rules)", result.surviving_rules.size());
        }
        return formatter<std::string_view>::format(text, ctx);
    }
};

}  // namespace fmt
#endif

#endif
