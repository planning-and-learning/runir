#include "detail.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

namespace runir::kr::ps::ext::dl::detail
{

ModuleIncompleteStructuralTerminationResult
make_incomplete_result(ModuleView module_, const ModuleAnalysis& analysis, const runir::kr::ps::detail::IncompletePolicyResult& policy_result)
{
    using SharedResult = runir::kr::ps::detail::IncompletePolicyResult;

    auto result = ModuleIncompleteStructuralTerminationResult {};
    const auto booleans = module_.get_features<runir::kr::ps::dl::BooleanFeature>();
    const auto numericals = module_.get_features<runir::kr::ps::dl::NumericalFeature>();
    for (const auto& shared_surviving : policy_result.surviving_rules)
    {
        auto surviving = IncompleteSurvivingRule { analysis.rules[shared_surviving.rule_position], {} };
        for (const auto& shared_reason : shared_surviving.blocking_reasons)
        {
            auto reason = IncompleteBlockingReason { shared_reason.feature_kind == SharedResult::FeatureKind::BOOLEAN ?
                                                         std::variant<BooleanFeatureView, NumericalFeatureView>(booleans[shared_reason.feature_position]) :
                                                         std::variant<BooleanFeatureView, NumericalFeatureView>(numericals[shared_reason.feature_position]),
                                                     {} };
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
