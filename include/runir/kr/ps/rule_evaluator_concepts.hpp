#ifndef RUNIR_KR_PS_RULE_EVALUATOR_CONCEPTS_HPP_
#define RUNIR_KR_PS_RULE_EVALUATOR_CONCEPTS_HPP_

#include "runir/kr/dl/semantics/state_evaluation_context.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <concepts>
#include <utility>

namespace runir::kr::ps
{

/// Family specializations borrow execution services and reuse prepared source planning views.
/// Storage is supplied by Ext; Base and ICP use their fixed execution services.
template<FamilyTag Family, tyr::TaskKind Kind, typename Storage = void, tyr::planning::StateViewConcept<Kind> PlanningState = tyr::planning::StateView<Kind>>
struct RuleEvaluationContext;

/// Context and State form a pair: the context must accept this family's source state and
/// provide its feature-evaluation resources for the same task kind. No state identity is required.
template<typename Context, typename Family, typename Kind, typename State>
concept RuleEvaluationContextConcept = FamilyTag<Family> && tyr::TaskKind<Kind> && std::copy_constructible<State> && requires(Context& context, State source) {
    requires std::same_as<typename Context::FamilyType, Family>;
    { context.make_dl_context(source) } -> runir::kr::dl::semantics::StateEvaluationContextConcept<DlFamilyFor<Family>, Kind>;
};

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

// Keep evaluator contracts organized around these two capabilities: emitting and matching.
// Reuse them across rule types. Adding another evaluator concept requires explicit
// confirmation from the developer before implementation.

/// Evaluate a source configuration through mutable callbacks; true means enumeration completed.
/// Result is the emitted value type (a step or a retained choice); its consumer controls continuation.
template<typename Evaluator, typename Family, typename Kind, typename Context, typename State, typename Result, typename Emit, typename Stop>
concept EmittingRuleEvaluatorConcept =
    RuleEvaluationContextConcept<Context, Family, Kind, State> && EmitConcept<Emit, Result> && StopConcept<Stop>
    && requires(const Evaluator& evaluator, Context& context, State source, Emit& emit, Stop& stop, bool (*emit_result)(Result)) {
           { evaluator.get_rule() } noexcept;
           { evaluator.emit(context, source, emit_result, stop) } -> std::same_as<bool>;
           { evaluator.emit(context, source, emit, stop) } -> std::same_as<bool>;
       };

/// Check a supplied candidate without publishing its action binding; scratch belongs to context.
template<typename Evaluator, typename Family, typename Kind, typename Context, typename State, typename Candidate>
concept MatchingRuleEvaluatorConcept = RuleEvaluationContextConcept<Context, Family, Kind, State>
                                       && requires(const Evaluator& evaluator, Context& context, State source, const Candidate& candidate) {
                                              {
                                                  context.make_dl_transition_context(source, candidate)
                                              } -> runir::kr::ps::dl::TransitionEvaluationContextConcept<Family, Kind>;
                                              { evaluator.get_rule() } noexcept;
                                              { evaluator.matches(context, source, candidate) } -> std::same_as<bool>;
                                          };

}  // namespace runir::kr::ps

#endif
