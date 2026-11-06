#ifndef RUNIR_KR_PS_EXT_DL_INCOMPLETE_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_EXT_DL_INCOMPLETE_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/kr/ps/ext/declarations.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <cstdint>
#include <variant>
#include <vector>

namespace runir::kr::ps::ext::dl
{

enum class IncompleteStructuralTerminationStatus : std::uint8_t
{
    TERMINATING,
    UNKNOWN,
};

struct IncompleteBlockingReason
{
    std::variant<BooleanFeatureView, NumericalFeatureView> feature;
    /// Remaining opponents in this rule's final residual memory component.
    urd::Store<urd::sequence<RuleVariantView>> opposing_rules;
};

struct IncompleteSurvivingRule
{
    RuleVariantView rule;
    urd::Store<urd::sequence<IncompleteBlockingReason>> blocking_reasons;
};

struct ModuleIncompleteStructuralTerminationResult
{
    IncompleteStructuralTerminationStatus status = IncompleteStructuralTerminationStatus::TERMINATING;
    /// Rules internal to final cyclic residual memory components.
    urd::Store<urd::sequence<IncompleteSurvivingRule>> surviving_rules;

    [[nodiscard]] bool is_terminating() const noexcept { return status == IncompleteStructuralTerminationStatus::TERMINATING; }
};

struct ProgramIncompleteStructuralTerminationResult
{
    IncompleteStructuralTerminationStatus status = IncompleteStructuralTerminationStatus::TERMINATING;
    urd::Store<urd::sequence<ModuleIncompleteStructuralTerminationResult>> module_results;
    urd::Store<urd::sequence<RuleVariantView>> recursive_call_rules;

    [[nodiscard]] bool is_terminating() const noexcept { return status == IncompleteStructuralTerminationStatus::TERMINATING; }
};

}  // namespace runir::kr::ps::ext::dl

#endif
