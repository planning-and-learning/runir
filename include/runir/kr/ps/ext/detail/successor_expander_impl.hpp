#ifndef RUNIR_KR_PS_EXT_DETAIL_SUCCESSOR_EXPANDER_IMPL_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SUCCESSOR_EXPANDER_IMPL_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/datasets/successors.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluators.hpp"
#include "runir/kr/ps/ext/successor_expander_decl.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <tyr/formalism/planning/action_view.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/nodes/labeled_node_view.hpp>
#include <tyr/planning/nodes/node_view.hpp>
#include <tyr/planning/states/state_repository.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::SuccessorExpander(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program) :
    m_task_context(task_context ? std::move(task_context) : throw std::invalid_argument("SuccessorExpander requires a task context.")),
    m_program(program),
    m_storage(m_task_context, m_program),
    m_rule_evaluators(m_task_context, m_program)
{
    if (&m_program.context() != m_task_context->domain_context->ext_repository.get())
        throw std::invalid_argument("SuccessorExpander requires a program from the domain context repository.");
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
auto SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::initial_state(const tyr::planning::StateBuilderView<Kind>& state) -> StoredState
{
    validate_planning_state(state);
    return m_storage.initial_state(state);
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
auto SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::start_state(ProgramStateView<Kind> state) -> StoredState
{
    validate_source(state);
    return m_storage.start_state(state);
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<urd::ViewOf<ProgramState<Kind>> S>
ProgramStateView<Kind> SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::materialize(const S& state)
{
    validate_source(state);
    return m_storage.materialize(state);
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<urd::ViewOf<ProgramState<Kind>> S, typename Emit, StopConcept Stop>
    requires EmitConcept<Emit, ProgramStep<ExtFamilyTag, Kind, StorageStoredState<ExecutionStorage>>>
             && EmitConcept<Emit, detail::Choice<runir::kr::dl::ConceptTag>> && EmitConcept<Emit, detail::Choice<runir::kr::dl::RoleTag>>
bool SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::for_each_successor(S state,
                                                                                     ProgramSearchStatistics& statistics,
                                                                                     const Emit& emit,
                                                                                     const Stop& stop)
{
    if (stop())
        return false;
    validate_source(state);
    get_environment().reset_source();
    get_environment().select_source(state);
    bool emitted = false;
    // Expansions pass through to emit: choices own their bindings and only move.
    const auto emit_expansion = [&]<typename Expansion>(Expansion&& expansion)
    {
        emitted = true;
        if constexpr (std::same_as<std::remove_cvref_t<Expansion>, Step>)
            statistics.num_generated += expansion.status == ProgramOutcome::APPLIED || expansion.status == ProgramOutcome::RESTORED_CALLER;
        return emit(std::forward<Expansion>(expansion));
    };
    auto context = m_rule_evaluators.make_context(m_storage, planning_state(state));
    if (!m_rule_evaluators.for_each_successor(context, state, emit_expansion, stop))
        return false;
    if (stop())
        return false;
    return emitted || emit_expansion(fallback(state, context.planning_state));
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<runir::kr::dl::CategoryTag Category, urd::ViewOf<ProgramState<Kind>> S>
auto SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::apply_choice(S state,
                                                                               const detail::Choice<Category>& choice,
                                                                               ProgramSearchStatistics& statistics) -> Step
{
    validate_source(state);
    auto context = m_rule_evaluators.make_context(m_storage, planning_state(state));
    auto step = m_rule_evaluators.apply_choice(context, state, choice);
    statistics.num_generated += step.status == ProgramOutcome::APPLIED;
    return step;
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
std::optional<RuleVariantView>
SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::matching_rule(ProgramStateView<Kind> state,
                                                                           const runir::datasets::LabeledPlanningNode<Kind>& candidate)
{
    validate_source(state);
    validate_planning_state(candidate.node().state());
    get_environment().reset_source();
    get_environment().select_source(state);
    get_environment().reset_target();
    auto context = m_rule_evaluators.make_context(m_storage, planning_state(state));
    return m_rule_evaluators.matching_rule(context, state, candidate);
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
auto SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::apply(StorageStateView<ExecutionStorage> state,
                                                                        RuleVariantView rule,
                                                                        std::optional<runir::datasets::LabeledPlanningNode<Kind>> candidate)
    -> std::optional<Step>
{
    validate_source(state);
    if (candidate)
        validate_planning_state(candidate->node().state());
    get_environment().reset_source();
    get_environment().select_source(state);
    get_environment().reset_target();
    auto context = m_rule_evaluators.make_context(m_storage, planning_state(state));
    return m_rule_evaluators.apply(context, state, rule, candidate);
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<tyr::planning::StateBuilderViewConcept<Kind> S>
void SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::validate_planning_state(const S& state) const
{
    if (&state.context() != m_task_context->search_context->state_repository.get())
        throw std::invalid_argument("SuccessorExpander requires a planning state from the selected task's state repository.");
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<urd::ViewOf<ProgramState<Kind>> S>
void SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::validate_source(S state) const
{
    // A state of another program is rejected where its module is selected (EvaluationEnvironment::select_source).
    if (&state.context() != m_task_context->execution_repository.get())
        throw std::invalid_argument("SuccessorExpander requires an execution state from the selected task.");
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<urd::ViewOf<ProgramState<Kind>> S, tyr::planning::StateBuilderViewConcept<Kind> PS>
auto SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::fallback(S state, const PS& planning_state) -> Step
{
    if (const auto caller = call_stack(state))
    {
        // Return: the caller's activation (at its return memory state) and its caller, over the current planning state.
        auto target = m_storage.store(planning_state, caller->module_state(), caller->caller());
        return detail::make_step<Kind, ExecutionStorage>(ProgramOutcome::RESTORED_CALLER, std::move(target), m_task_context);
    }
    return detail::make_step<Kind, ExecutionStorage>(ProgramOutcome::NO_APPLICABLE_ACTION, m_storage.retain(state), m_task_context);
}

}  // namespace runir::kr::ps::ext

#endif
