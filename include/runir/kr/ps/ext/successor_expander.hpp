#ifndef RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluators.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
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
#include <tyr/planning/state_view.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/containers/variant.hpp>

namespace runir::kr::ps::ext
{

/// Returned choices must not outlive this expander.
/// With TransientExecutionStorage, returned states and steps must not outlive it either.
template<tyr::TaskKind Kind, typename ExecutionStorage = InternedExecutionStorage<Kind>>
class SuccessorExpander
{
public:
    SuccessorExpander(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program) :
        m_task_context(task_context ? std::move(task_context) : throw std::invalid_argument("SuccessorExpander requires a task context.")),
        m_program(program),
        m_storage(m_task_context, m_program),
        m_rule_evaluators(m_task_context, m_program)
    {
        if (&m_program.get_context() != m_task_context->domain_context->ext_repository.get())
            throw std::invalid_argument("SuccessorExpander requires a program from the domain context repository.");
    }

    const auto& get_task_context() const noexcept { return m_task_context; }
    auto& get_environment() noexcept { return m_rule_evaluators.get_environment(); }

    template<StoredProgramStateConcept<Kind> S>
    auto view(const S& state) const
    {
        return m_storage.view(state);
    }

    /// Create the entry module with empty registers and arguments, and no caller.
    auto initial_state(const tyr::planning::StateView<Kind>& state)
    {
        validate_planning_state(state);
        return m_storage.initial_state(state);
    }

    /// Intern selected output states and, in CHOICE mode, sources with Choose obligations.
    template<ProgramStateViewConcept<Kind> S>
    ProgramStateView<Kind> materialize(const S& state)
    {
        validate_source(state);
        return m_storage.materialize(state);
    }

    /// Emit a program step or a compact Choose obligation in natural rule and binding order,
    /// except that sketch rules with effects are emitted together, binding-major, after all other rules.
    /// Return true on exhaustion; emit returning false or stop returning true ends enumeration.
    /// Count applied successors and caller returns, not Choice descriptors or failure markers.
    /// Callbacks may materialize the source, but must not generate successors or apply choices. Apply choices after enumeration.
    template<ProgramStateViewConcept<Kind> S, typename Emit, typename Stop>
    bool for_each_successor(S state, ProgramSearchStatistics& statistics, Emit&& emit, Stop&& stop)
    {
        using Step = detail::ProgramStep<Kind, decltype(m_storage.retain(state))>;
        if (stop())
            return false;
        validate_source(state);
        get_environment().reset_source();
        bool emitted = false;
        const auto emit_expansion = [&](auto expansion)
        {
            emitted = true;
            if constexpr (std::same_as<decltype(expansion), Step>)
                statistics.num_generated += expansion.status == detail::ProgramOutcome::APPLIED || expansion.status == detail::ProgramOutcome::RESTORED_CALLER;
            return emit(std::move(expansion));
        };
        auto context = evaluation_context();
        if (!m_rule_evaluators.for_each_successor(context, state, emit_expansion, stop))
            return false;
        if (stop())
            return false;
        return emitted || emit_expansion(fallback(state));
    }

    /// Apply the current admitted binding without advancing its cursor; an exhausted choice reports FAILURE.
    template<runir::kr::dl::CategoryTag Category, ProgramStateViewConcept<Kind> S>
    auto apply_choice(S state, const detail::Choice<Category>& choice, ProgramSearchStatistics& statistics)
    {
        validate_source(state);
        auto context = evaluation_context();
        auto step = m_rule_evaluators.apply_choice(context, state, choice);
        statistics.num_generated += step.status == detail::ProgramOutcome::APPLIED;
        return step;
    }

    /// Find the first rule that admits this planning successor; control-only rules do not match planning actions.
    std::optional<RuleVariantView> matching_rule(ProgramStateView<Kind> state, const tyr::planning::LabeledNode<tyr::planning::StateView<Kind>>& candidate)
    {
        validate_source(state);
        validate_planning_state(candidate.node.get_state());
        get_environment().reset_source();
        get_environment().reset_target();
        auto context = evaluation_context();
        return m_rule_evaluators.matching_rule(context, state, candidate);
    }

    /// Apply one rule, using a supplied planning successor for Do, Action, or a Sketch with effects.
    /// Load, Choose, Call, and empty-effect Sketch rules derive their own control transition.
    std::optional<detail::ProgramStep<Kind>> apply(ProgramStateView<Kind> state,
                                                   RuleVariantView rule,
                                                   std::optional<tyr::planning::LabeledNode<tyr::planning::StateView<Kind>>> candidate = std::nullopt)
    {
        validate_source(state);
        if (candidate)
            validate_planning_state(candidate->node.get_state());
        get_environment().reset_source();
        get_environment().reset_target();
        auto context = evaluation_context();
        return m_rule_evaluators.apply(context, state, rule, candidate);
    }

private:
    // Validate borrowed views before evaluating features or modifying the execution repository.
    template<tyr::planning::StateViewConcept<Kind> S>
    void validate_planning_state(const S& state) const
    {
        if constexpr (requires { state.get_state_repository(); })
        {
            if (state.get_state_repository().get() != m_task_context->search_context->state_repository.get())
                throw std::invalid_argument("SuccessorExpander requires a planning state from the selected task's state repository.");
        }
        else if (&state.get_task() != m_task_context->search_context->task.get())
            throw std::invalid_argument("SuccessorExpander requires a planning state from the selected task.");
    }

    template<ProgramStateViewConcept<Kind> S>
    void validate_source(S state) const
    {
        if (&state.get_context() != m_task_context->execution_repository.get())
            throw std::invalid_argument("SuccessorExpander requires an execution state from the selected task.");
        const auto program = state.get_program();
        if (&program.get_context() != &m_program.get_context() || program.get_index() != m_program.get_index())
            throw std::invalid_argument("SuccessorExpander requires an execution state from the selected program.");
    }

    auto evaluation_context() { return detail::RuleEvaluationContext<Kind, ExecutionStorage> { m_task_context, m_storage, get_environment() }; }

    /// With no emitted rule outcome, return to the caller or report an open top-level state.
    /// Restore caller control and bindings while retaining the planning state reached by the callee.
    template<ProgramStateViewConcept<Kind> S>
    auto fallback(S state)
    {
        if (const auto caller = state.get_call_stack())
        {
            auto target = ext::make_module(m_storage,
                                           state.get_state(),
                                           caller->get_module(),
                                           caller->get_return_memory_state(),
                                           caller->get_registers(),
                                           caller->get_arguments());
            return detail::make_step(detail::ProgramOutcome::RESTORED_CALLER, m_storage.store(std::move(target), caller->get_caller()), m_task_context);
        }
        return detail::make_step(detail::ProgramOutcome::NO_APPLICABLE_ACTION, m_storage.retain(state), m_task_context);
    }

    runir::kr::TaskContextPtr<Kind> m_task_context;
    ProgramView m_program;
    ExecutionStorage m_storage;
    detail::RuleEvaluators<Kind> m_rule_evaluators;
};

template<tyr::TaskKind Kind>
using TransientSuccessorExpander = SuccessorExpander<Kind, TransientExecutionStorage<Kind>>;

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class SuccessorExpander<tyr::GroundTag>;
extern template class SuccessorExpander<tyr::LiftedTag>;

extern template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag>::materialize<ProgramStateView<tyr::GroundTag>>(const ProgramStateView<tyr::GroundTag>&);

extern template auto
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

extern template auto
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

extern template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::materialize<BuilderProgramStateView<tyr::GroundTag>>(
    const BuilderProgramStateView<tyr::GroundTag>&);

extern template auto
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

extern template auto
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::RoleTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

extern template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag>::materialize<ProgramStateView<tyr::LiftedTag>>(const ProgramStateView<tyr::LiftedTag>&);

extern template auto
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

extern template auto
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

extern template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::materialize<BuilderProgramStateView<tyr::LiftedTag>>(
    const BuilderProgramStateView<tyr::LiftedTag>&);

extern template auto
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::LiftedTag>>(
    BuilderProgramStateView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

extern template auto
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::RoleTag, BuilderProgramStateView<tyr::LiftedTag>>(
    BuilderProgramStateView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

#endif

}  // namespace runir::kr::ps::ext

#endif
