#ifndef RUNIR_KR_PS_RULE_EVALUATOR_CONCEPTS_HPP_
#define RUNIR_KR_PS_RULE_EVALUATOR_CONCEPTS_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/evaluation_policy.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <concepts>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::ps
{

/// Family specializations borrow execution services and reuse prepared source planning views.
/// Storage is supplied by Ext; Base and ICP use their fixed execution services.
template<FamilyTag Family,
         tyr::TaskKind Kind,
         typename Storage = void,
         tyr::planning::StateViewConcept<Kind> PlanningState = tyr::planning::StateView<Kind>,
         runir::kr::dl::semantics::EvaluationPolicyConcept<DlFamilyFor<Family>, Kind> EvaluationPolicy =
             runir::kr::dl::semantics::DefaultEvaluationPolicy<DlFamilyFor<Family>, Kind>>
struct RuleEvaluationContext;

/// Consume a result; false requests early termination. The callback may mutate search state.
template<typename Emit, typename Result>
concept EmitConcept = requires(Emit& emit, Result result) {
    { emit(std::move(result)) } -> std::same_as<bool>;
};

/// Poll for cancellation; true requests termination. The callback may change between polls.
template<typename Stop>
concept StopConcept = requires(Stop& stop) {
    { stop() } -> std::same_as<bool>;
};

}  // namespace runir::kr::ps

#endif
