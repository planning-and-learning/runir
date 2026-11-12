#ifndef RUNIR_KR_PS_DL_INCOMPLETE_STRUCTURAL_TERMINATION_DATA_HPP_
#define RUNIR_KR_PS_DL_INCOMPLETE_STRUCTURAL_TERMINATION_DATA_HPP_

#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <cstdint>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <variant>

namespace runir::kr::ps::dl
{

enum class IncompleteStructuralTerminationStatus : std::uint8_t
{
    TERMINATING,
    UNKNOWN,
};

/// Why one elimination attempt for a surviving rule is blocked.
template<FamilyTag Family>
struct IncompleteBlockingReason
{
    /// The feature whose strict change (numerical) or flip (Boolean) the rule
    /// performs.
    std::variant<FeatureView<Family, runir::kr::dl::BooleanTag>, FeatureView<Family, runir::kr::dl::NumericalTag>> feature;
    /// The remaining rules that oppose the change and survive R3's
    /// marked-complementary-condition check; for a module, the opponents in the
    /// rule's final residual memory component.
    urd::Store<urd::sequence<RuleView<Family>>> opposing_rules;
};

template<FamilyTag Family>
struct IncompleteSurvivingRule
{
    RuleView<Family> rule;
    /// One entry per feature the rule changes strictly (numerical) or flips
    /// (Boolean); empty iff the rule performs no eliminating change at all.
    urd::Store<urd::sequence<IncompleteBlockingReason<Family>>> blocking_reasons;
};

/// The incomplete verdict on a sketch (base) or on one module (ext).
template<FamilyTag Family>
struct IncompleteStructuralTerminationResult
{
    IncompleteStructuralTerminationStatus status = IncompleteStructuralTerminationStatus::TERMINATING;
    /// Rules internal to final cyclic residual (memory) components; empty iff terminating.
    urd::Store<urd::sequence<IncompleteSurvivingRule<Family>>> surviving_rules;

    [[nodiscard]] bool is_terminating() const noexcept { return status == IncompleteStructuralTerminationStatus::TERMINATING; }
};

}  // namespace runir::kr::ps::dl

#endif
