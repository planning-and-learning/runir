#ifndef RUNIR_KR_PS_EXT_DETAIL_SUCCESSOR_EXPANDER_IMPL_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SUCCESSOR_EXPANDER_IMPL_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluators.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"
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
#include <tyr/planning/node.hpp>
#include <tyr/planning/state_repository.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/containers/variant.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::SuccessorExpander(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program) :
    m_task_context(task_context ? std::move(task_context) : throw std::invalid_argument("SuccessorExpander requires a task context.")),
    m_program(program),
    m_storage(m_task_context, m_program),
    m_rule_evaluators(m_task_context, m_program)
{
    if (&m_program.get_context() != m_task_context->domain_context->ext_repository.get())
        throw std::invalid_argument("SuccessorExpander requires a program from the domain context repository.");
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
auto SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::initial_state(const tyr::planning::StateView<Kind>& state) -> StoredState
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
template<ProgramStateViewConcept<Kind> S>
ProgramStateView<Kind> SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::materialize(const S& state)
{
    validate_source(state);
    return m_storage.materialize(state);
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<ProgramStateViewConcept<Kind> S, typename Emit, StopConcept Stop>
    requires EmitConcept<Emit, detail::ProgramStep<Kind, ExecutionStorage>> && EmitConcept<Emit, detail::Choice<runir::kr::dl::ConceptTag>>
             && EmitConcept<Emit, detail::Choice<runir::kr::dl::RoleTag>>
bool SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::for_each_successor(S state, ProgramSearchStatistics& statistics, Emit&& emit, Stop&& stop)
{
    if (stop())
        return false;
    validate_source(state);
    get_environment().reset_source();
    get_environment().select_source(state);
    bool emitted = false;
    const auto emit_expansion = [&]<typename Expansion>(Expansion expansion)
    {
        emitted = true;
        if constexpr (std::same_as<Expansion, Step>)
            statistics.num_generated += expansion.status == detail::ProgramOutcome::APPLIED || expansion.status == detail::ProgramOutcome::RESTORED_CALLER;
        return emit(std::move(expansion));
    };
    auto context = m_rule_evaluators.make_context(m_storage, state.get_state());
    if (!m_rule_evaluators.for_each_successor(context, state, emit_expansion, stop))
        return false;
    if (stop())
        return false;
    return emitted || emit_expansion(fallback(state, context.planning_state));
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<runir::kr::dl::CategoryTag Category, ProgramStateViewConcept<Kind> S>
auto SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::apply_choice(S state,
                                                                               const detail::Choice<Category>& choice,
                                                                               ProgramSearchStatistics& statistics) -> Step
{
    validate_source(state);
    auto context = m_rule_evaluators.make_context(m_storage, state.get_state());
    auto step = m_rule_evaluators.apply_choice(context, state, choice);
    statistics.num_generated += step.status == detail::ProgramOutcome::APPLIED;
    return step;
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
std::optional<RuleVariantView> SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::matching_rule(ProgramStateView<Kind> state,
                                                                                                          const tyr::planning::LabeledNode<Kind>& candidate)
{
    validate_source(state);
    validate_planning_state(candidate.node.get_state());
    get_environment().reset_source();
    get_environment().select_source(state);
    get_environment().reset_target();
    auto context = m_rule_evaluators.make_context(m_storage, state.get_state());
    return m_rule_evaluators.matching_rule(context, state, candidate);
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
auto SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::apply(StorageStateView<ExecutionStorage> state,
                                                                        RuleVariantView rule,
                                                                        std::optional<tyr::planning::LabeledNode<Kind>> candidate) -> std::optional<Step>
{
    validate_source(state);
    if (candidate)
        validate_planning_state(candidate->node.get_state());
    get_environment().reset_source();
    get_environment().select_source(state);
    get_environment().reset_target();
    auto context = m_rule_evaluators.make_context(m_storage, state.get_state());
    return m_rule_evaluators.apply(context, state, rule, candidate);
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<tyr::planning::StateViewConcept<Kind> S>
void SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::validate_planning_state(const S& state) const
{
    if constexpr (requires { state.get_state_repository(); })
    {
        if (state.get_state_repository().get() != m_task_context->search_context->state_repository.get())
            throw std::invalid_argument("SuccessorExpander requires a planning state from the selected task's state repository.");
    }
    else if (&state.get_task() != m_task_context->search_context->task.get())
        throw std::invalid_argument("SuccessorExpander requires a planning state from the selected task.");
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<ProgramStateViewConcept<Kind> S>
void SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::validate_source(S state) const
{
    if (&state.get_context() != m_task_context->execution_repository.get())
        throw std::invalid_argument("SuccessorExpander requires an execution state from the selected task.");
    const auto program = state.get_program();
    if (&program.get_context() != &m_program.get_context() || program != m_program)
        throw std::invalid_argument("SuccessorExpander requires an execution state from the selected program.");
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
template<ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
auto SuccessorExpander<Kind, ExecutionStorage, EvaluationPolicy>::fallback(S state, const PS& planning_state) -> Step
{
    if (const auto caller = state.get_call_stack())
    {
        auto target = m_storage.store(planning_state,
                                      caller->get_module(),
                                      caller->get_return_memory_state(),
                                      caller->get_registers(),
                                      caller->get_arguments(),
                                      caller->get_caller());
        return detail::make_step<Kind, ExecutionStorage>(detail::ProgramOutcome::RESTORED_CALLER, std::move(target), m_task_context);
    }
    return detail::make_step<Kind, ExecutionStorage>(detail::ProgramOutcome::NO_APPLICABLE_ACTION, m_storage.retain(state), m_task_context);
}

}  // namespace runir::kr::ps::ext

#endif
