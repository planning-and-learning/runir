#include "detail.hpp"

namespace runir::kr::ps::base::dl::detail
{

IncompleteStructuralTerminationResult make_incomplete_result(SketchView sketch, const SketchAnalysis& analysis, const IncompleteSieveResult& sieve_result)
{
    auto result = IncompleteStructuralTerminationResult {};
    result.booleans = analysis.features.booleans;
    result.numericals = analysis.features.numericals;

    for (std::size_t rule_position = 0; rule_position < analysis.rules.size(); ++rule_position)
    {
        if (!sieve_result.remaining[rule_position])
            continue;

        auto surviving = IncompleteSurvivingRule { analysis.rules[rule_position], {} };
        for (std::size_t position = 0; position < analysis.features.numericals.size(); ++position)
        {
            if (!sieve_result.changes[rule_position].numerical_decreases.test(position))
                continue;
            auto reason = IncompleteBlockingReason { NumericalFeatureView(analysis.features.numericals[position], sketch.get_context()), {} };
            for (auto other : opposing_rules(analysis, sieve_result, rule_position, false, position, false))
                reason.opposing_rules.push_back(analysis.rules[other]);
            surviving.blocking_reasons.push_back(std::move(reason));
        }
        for (std::size_t position = 0; position < analysis.features.booleans.size(); ++position)
        {
            const auto to_true = sieve_result.changes[rule_position].boolean_to_true.test(position);
            const auto to_false = sieve_result.changes[rule_position].boolean_to_false.test(position);
            if (!to_true && !to_false)
                continue;
            auto reason = IncompleteBlockingReason { BooleanFeatureView(analysis.features.booleans[position], sketch.get_context()), {} };
            for (auto other : opposing_rules(analysis, sieve_result, rule_position, true, position, to_true))
                reason.opposing_rules.push_back(analysis.rules[other]);
            surviving.blocking_reasons.push_back(std::move(reason));
        }
        result.surviving_rules.push_back(std::move(surviving));
    }

    result.status = result.surviving_rules.empty() ? IncompleteStructuralTerminationStatus::TERMINATING : IncompleteStructuralTerminationStatus::UNKNOWN;
    return result;
}

}  // namespace runir::kr::ps::base::dl::detail
