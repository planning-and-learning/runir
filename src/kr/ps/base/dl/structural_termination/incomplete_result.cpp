#include "detail.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"

namespace runir::kr::ps::base::dl::detail
{

IncompleteStructuralTerminationResult
make_incomplete_result(SketchView sketch, const SketchAnalysis& analysis, const runir::kr::ps::detail::IncompletePolicyResult& policy_result)
{
    auto result = IncompleteStructuralTerminationResult {};
    const auto booleans = sketch.get_features<runir::kr::ps::dl::BooleanFeature>();
    const auto numericals = sketch.get_features<runir::kr::ps::dl::NumericalFeature>();

    for (const auto& policy_rule : policy_result.surviving_rules)
    {
        auto surviving = IncompleteSurvivingRule { analysis.rules[policy_rule.rule_position], {} };
        for (const auto& policy_reason : policy_rule.blocking_reasons)
        {
            auto reason = IncompleteBlockingReason { policy_reason.feature_kind == runir::kr::ps::detail::IncompletePolicyResult::FeatureKind::BOOLEAN ?
                                                         std::variant<BooleanFeatureView, NumericalFeatureView>(booleans[policy_reason.feature_position]) :
                                                         std::variant<BooleanFeatureView, NumericalFeatureView>(numericals[policy_reason.feature_position]),
                                                     {} };
            for (const auto other : policy_reason.opposing_rule_positions)
                reason.opposing_rules.push_back(analysis.rules[other]);
            surviving.blocking_reasons.push_back(std::move(reason));
        }
        result.surviving_rules.push_back(std::move(surviving));
    }

    result.status = policy_result.is_terminating() ? IncompleteStructuralTerminationStatus::TERMINATING : IncompleteStructuralTerminationStatus::UNKNOWN;
    return result;
}

}  // namespace runir::kr::ps::base::dl::detail
