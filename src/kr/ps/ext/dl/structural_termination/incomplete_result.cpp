#include "detail.hpp"

namespace runir::kr::ps::ext::dl::detail
{

ModuleIncompleteStructuralTerminationResult make_incomplete_result(const ModuleAnalysis& analysis,
                                                                   const runir::kr::ps::detail::IncompletePolicyResult& policy_result)
{
    using SharedResult = runir::kr::ps::detail::IncompletePolicyResult;

    auto result = ModuleIncompleteStructuralTerminationResult {};
    result.booleans = analysis.features.booleans;
    result.numericals = analysis.features.numericals;
    for (const auto& shared_surviving : policy_result.surviving_rules)
    {
        auto surviving = IncompleteSurvivingRule { analysis.rules[shared_surviving.rule_position], {} };
        for (const auto& shared_reason : shared_surviving.blocking_reasons)
        {
            auto reason = IncompleteBlockingReason {
                shared_reason.feature_kind == SharedResult::FeatureKind::BOOLEAN ?
                    std::variant<BooleanFeatureView, NumericalFeatureView>(analysis.features.booleans[shared_reason.feature_position]) :
                    std::variant<BooleanFeatureView, NumericalFeatureView>(analysis.features.numericals[shared_reason.feature_position]),
                {}
            };
            for (const auto opposing_position : shared_reason.opposing_rule_positions)
                reason.opposing_rules.push_back(analysis.rules[opposing_position]);
            surviving.blocking_reasons.push_back(std::move(reason));
        }
        result.surviving_rules.push_back(std::move(surviving));
    }
    if (!result.surviving_rules.empty())
        result.status = IncompleteStructuralTerminationStatus::UNKNOWN;
    return result;
}

}  // namespace runir::kr::ps::ext::dl::detail
