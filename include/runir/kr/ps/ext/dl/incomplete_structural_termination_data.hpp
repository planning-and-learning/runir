#ifndef RUNIR_KR_PS_EXT_DL_INCOMPLETE_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_EXT_DL_INCOMPLETE_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/kr/ps/ext/repository.hpp"

#include <variant>
#include <vector>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext::dl
{

enum class IncompleteStructuralTerminationStatus
{
    TERMINATING,
    UNKNOWN,
};

using BooleanFeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::BooleanFeature>>, Repository>;
using NumericalFeatureView = ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, runir::kr::ps::dl::NumericalFeature>>, Repository>;

struct IncompleteBlockingReason
{
    std::variant<BooleanFeatureView, NumericalFeatureView> feature;
    /// Remaining opponents in this rule's final residual memory component.
    std::vector<RuleVariantView> opposing_rules;
};

struct IncompleteSurvivingRule
{
    RuleVariantView rule;
    std::vector<IncompleteBlockingReason> blocking_reasons;
};

struct ModuleIncompleteStructuralTerminationResult
{
    IncompleteStructuralTerminationStatus status = IncompleteStructuralTerminationStatus::TERMINATING;
    std::vector<BooleanFeatureView> booleans;
    std::vector<NumericalFeatureView> numericals;
    /// Rules internal to final cyclic residual memory components.
    std::vector<IncompleteSurvivingRule> surviving_rules;

    bool is_terminating() const noexcept { return status == IncompleteStructuralTerminationStatus::TERMINATING; }
};

struct ModuleProgramIncompleteStructuralTerminationResult
{
    IncompleteStructuralTerminationStatus status = IncompleteStructuralTerminationStatus::TERMINATING;
    std::vector<ModuleIncompleteStructuralTerminationResult> module_results;
    std::vector<RuleVariantView> recursive_call_rules;

    bool is_terminating() const noexcept { return status == IncompleteStructuralTerminationStatus::TERMINATING; }
};

}  // namespace runir::kr::ps::ext::dl

#endif
