#ifndef RUNIR_KR_PS_BASE_DL_INCOMPLETE_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_BASE_DL_INCOMPLETE_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/kr/ps/base/repository.hpp"

#include <variant>
#include <vector>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::base::dl
{

enum class IncompleteStructuralTerminationStatus
{
    TERMINATING,
    UNKNOWN,
};

using BooleanFeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>, Repository>;
using NumericalFeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>, Repository>;

/// Why one elimination attempt for a surviving rule is blocked.
struct IncompleteBlockingReason
{
    /// The feature whose decrease (numerical) or flip (Boolean) the rule
    /// performs.
    std::variant<BooleanFeatureView, NumericalFeatureView> feature;
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

}  // namespace runir::kr::ps::base::dl

#endif
