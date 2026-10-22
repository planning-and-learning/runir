#ifndef RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/evaluation_policy.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluators.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <optional>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node_concepts.hpp>
#include <tyr/planning/state_view_concepts.hpp>

namespace runir::kr::ps::ext
{

/// Returned choices must not outlive this expander.
/// With TransientExecutionStorage, returned states and steps must not outlive it either.
template<tyr::TaskKind Kind,
         ExecutionStorageConcept<Kind> ExecutionStorage = InternedExecutionStorage<Kind>,
         runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy = runir::kr::dl::semantics::DefaultEvaluationPolicy<Kind>>
class SuccessorExpander
{
public:
    using StorageType = ExecutionStorage;
    using StoredState = StorageStoredState<ExecutionStorage>;
    using Step = detail::ProgramStep<Kind, ExecutionStorage>;

    SuccessorExpander(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program);

    const runir::kr::TaskContextPtr<Kind>& get_task_context() const noexcept { return m_task_context; }
    EvaluationEnvironment<Kind, EvaluationPolicy>& get_environment() noexcept { return m_rule_evaluators.get_environment(); }

    StorageStateView<ExecutionStorage> view(const StoredState& state) const { return m_storage.view(state); }

    /// Create the entry module with empty registers and arguments, and no caller.
    StoredState initial_state(const tyr::planning::StateView<Kind>& state);

    /// Intern selected output states and, in CHOICE mode, sources with Choose obligations.
    template<ProgramStateViewConcept<Kind> S>
    ProgramStateView<Kind> materialize(const S& state);

    /// Emit a program step or a compact Choose obligation in natural rule and binding order.
    /// A matching Backtrack emits only failure; effectful Sketch rules are batched after other ordinary rules.
    /// Return true on exhaustion; emit returning false or stop returning true ends enumeration.
    /// Count applied successors and caller returns, not Choice descriptors or failure markers.
    /// Callbacks may materialize the source, but must not generate successors or apply choices. Apply choices after enumeration.
    template<ProgramStateViewConcept<Kind> S, typename Emit, StopConcept Stop>
        requires EmitConcept<Emit, detail::ProgramStep<Kind, ExecutionStorage>> && EmitConcept<Emit, detail::Choice<runir::kr::dl::ConceptTag>>
                 && EmitConcept<Emit, detail::Choice<runir::kr::dl::RoleTag>>
    bool for_each_successor(S state, ProgramSearchStatistics& statistics, Emit&& emit, Stop&& stop);

    /// Apply the current admitted binding without advancing its cursor; an exhausted choice reports FAILURE.
    template<runir::kr::dl::CategoryTag Category, ProgramStateViewConcept<Kind> S>
    Step apply_choice(S state, const detail::Choice<Category>& choice, ProgramSearchStatistics& statistics);

    /// Find the first rule that admits this planning successor; control-only rules do not match planning actions.
    std::optional<RuleVariantView> matching_rule(ProgramStateView<Kind> state, const tyr::planning::LabeledNode<Kind>& candidate);

    /// Apply one rule, using a supplied planning successor for Do, Action, or a Sketch with effects.
    /// Load, Choose, Call, and empty-effect Sketch rules derive their own control transition.
    /// Backtrack emits failure without requiring a planning successor.
    std::optional<Step>
    apply(StorageStateView<ExecutionStorage> state, RuleVariantView rule, std::optional<tyr::planning::LabeledNode<Kind>> candidate = std::nullopt);

private:
    // Validate borrowed views before evaluating features or modifying the execution repository.
    template<tyr::planning::StateViewConcept<Kind> S>
    void validate_planning_state(const S& state) const;

    template<ProgramStateViewConcept<Kind> S>
    void validate_source(S state) const;

    /// With no emitted rule outcome, return to the caller or report an open top-level state.
    /// Restore caller control and bindings while retaining the planning state reached by the callee.
    template<ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    Step fallback(S state, const PS& planning_state);

    runir::kr::TaskContextPtr<Kind> m_task_context;
    ProgramView m_program;
    ExecutionStorage m_storage;
    detail::RuleEvaluators<Kind, EvaluationPolicy> m_rule_evaluators;
};

template<tyr::TaskKind Kind>
using TransientSuccessorExpander = SuccessorExpander<Kind, TransientExecutionStorage<Kind>>;

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class SuccessorExpander<tyr::GroundTag>;
extern template class SuccessorExpander<tyr::LiftedTag>;

extern template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag>::materialize<ProgramStateView<tyr::GroundTag>>(const ProgramStateView<tyr::GroundTag>&);

extern template detail::ProgramStep<tyr::GroundTag, InternedExecutionStorage<tyr::GroundTag>>
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

extern template detail::ProgramStep<tyr::GroundTag, InternedExecutionStorage<tyr::GroundTag>>
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

extern template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::materialize<BuilderProgramStateView<tyr::GroundTag>>(
    const BuilderProgramStateView<tyr::GroundTag>&);

extern template detail::ProgramStep<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

extern template detail::ProgramStep<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::RoleTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

extern template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag>::materialize<ProgramStateView<tyr::LiftedTag>>(const ProgramStateView<tyr::LiftedTag>&);

extern template detail::ProgramStep<tyr::LiftedTag, InternedExecutionStorage<tyr::LiftedTag>>
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

extern template detail::ProgramStep<tyr::LiftedTag, InternedExecutionStorage<tyr::LiftedTag>>
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

extern template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::materialize<BuilderProgramStateView<tyr::LiftedTag>>(
    const BuilderProgramStateView<tyr::LiftedTag>&);

extern template detail::ProgramStep<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::LiftedTag>>(
    BuilderProgramStateView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

extern template detail::ProgramStep<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::RoleTag, BuilderProgramStateView<tyr::LiftedTag>>(
    BuilderProgramStateView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

#endif

}  // namespace runir::kr::ps::ext

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/ps/ext/detail/successor_expander_impl.hpp"
#endif

#endif
