#ifndef RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/ext/state_evaluation_context.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"
#include "runir/kr/ps/ext/detail/transient_values.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/task_context.hpp"

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext
{

namespace detail
{

template<tyr::TaskKind Kind>
class TransientEvaluationContext : public runir::kr::dl::semantics::BaseStateEvaluationContext<ExtFamilyTag, Kind, tyr::planning::BuilderStateView<Kind>>
{
    using Base = runir::kr::dl::semantics::BaseStateEvaluationContext<ExtFamilyTag, Kind, tyr::planning::BuilderStateView<Kind>>;
    CallArgumentsRef m_arguments;
    RegisterValuesRef m_registers;

public:
    TransientEvaluationContext(tyr::planning::BuilderStateView<Kind> state,
                               runir::kr::dl::semantics::Builder& builder,
                               runir::kr::dl::semantics::DenotationRepository& repository,
                               runir::kr::dl::semantics::EvaluationWorkspace& workspace,
                               runir::kr::dl::semantics::DenotationCaches<ExtFamilyTag>& caches,
                               CallArgumentsRef arguments,
                               RegisterValuesRef registers) :
        Base(state, builder, repository, workspace, caches),
        m_arguments(arguments),
        m_registers(registers)
    {
    }
    auto arguments() const noexcept { return m_arguments; }
    auto registers() const noexcept { return m_registers; }
};

template<tyr::TaskKind Kind>
struct TransientTransitionContext
{
    using FamilyType = ExtFamilyTag;
    TransientEvaluationContext<Kind> source;
    TransientEvaluationContext<Kind> target;
    auto& get_source_context() noexcept { return source; }
    auto& get_target_context() noexcept { return target; }
    const auto& get_source_state() const noexcept { return source.get_state(); }
    const auto& get_target_state() const noexcept { return target.get_state(); }
};

}  // namespace detail

template<tyr::TaskKind Kind>
class EvaluationEnvironment
{
private:
    using StateDlContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, Kind>;
    using TransitionDlContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::ExtFamilyTag, Kind>;

    runir::kr::dl::semantics::Builder& m_dl_builder;
    runir::kr::dl::semantics::DenotationRepository& m_dl_denotation_repository;
    ExecutionRepository<Kind>& m_execution_repository;
    runir::kr::dl::semantics::EvaluationWorkspace m_dl_workspace;
    runir::kr::dl::semantics::DenotationCaches<runir::kr::ExtFamilyTag> m_dl_caches;
    runir::kr::dl::semantics::DenotationCaches<runir::kr::ExtFamilyTag> m_dl_target_caches;
    std::vector<runir::kr::dl::semantics::ConceptDenotationView> m_do_argument_denotations;
    ProgramView m_program;

public:
    EvaluationEnvironment(runir::kr::TaskContext<Kind>& task_context, ProgramView program) :
        m_dl_builder(task_context.dl_builder),
        m_dl_denotation_repository(*task_context.dl_denotation_repository),
        m_execution_repository(*task_context.execution_repository),
        m_dl_workspace(),
        m_dl_caches(m_dl_denotation_repository),
        m_dl_target_caches(m_dl_denotation_repository),
        m_do_argument_denotations(),
        m_program(program)
    {
    }

    auto get_program() const noexcept { return m_program; }
    auto& get_dl_repository() noexcept { return m_program.get_context().get_dl_repository(); }
    auto& get_dl_workspace() noexcept { return m_dl_workspace; }
    auto& get_dl_caches() noexcept { return m_dl_caches; }
    auto& get_dl_target_caches() noexcept { return m_dl_target_caches; }

    auto& prepare_do_argument_denotations()
    {
        m_do_argument_denotations.clear();
        return m_do_argument_denotations;
    }

    /// Contexts borrow stored register and argument data. Cache invalidation remains explicit.
    StateDlContext make_dl_context(ProgramStateView<Kind> state)
    {
        const auto program = state.get_program();
        if (&state.get_context() != &m_execution_repository || &program.get_context() != &m_program.get_context()
            || program.get_index() != m_program.get_index())
            throw std::invalid_argument("EvaluationEnvironment requires an execution state from the selected task and program.");
        const auto module_state = state.get_module_state();
        return make_dl_context(module_state.get_state(), module_state.get_arguments(), module_state.get_registers());
    }

    StateDlContext make_dl_context(tyr::planning::StateView<Kind> state,
                                   runir::kr::dl::semantics::CallArgumentsView arguments,
                                   runir::kr::dl::semantics::RegisterValuesView registers)
    {
        return StateDlContext(std::move(state), m_dl_builder, m_dl_denotation_repository, m_dl_workspace, m_dl_caches, arguments, registers);
    }

    auto make_dl_context(tyr::planning::BuilderStateView<Kind> state, detail::CallArgumentsRef arguments, detail::RegisterValuesRef registers)
    {
        return detail::TransientEvaluationContext<Kind>(state, m_dl_builder, m_dl_denotation_repository, m_dl_workspace, m_dl_caches, arguments, registers);
    }

    auto make_dl_transition_context(tyr::planning::BuilderStateView<Kind> source_state,
                                    tyr::planning::BuilderStateView<Kind> target_state,
                                    detail::CallArgumentsRef arguments,
                                    detail::RegisterValuesRef source_registers,
                                    detail::RegisterValuesRef target_registers)
    {
        return detail::TransientTransitionContext<Kind> { make_dl_context(source_state, arguments, source_registers),
                                                          detail::TransientEvaluationContext<Kind>(target_state,
                                                                                                   m_dl_builder,
                                                                                                   m_dl_denotation_repository,
                                                                                                   m_dl_workspace,
                                                                                                   m_dl_target_caches,
                                                                                                   arguments,
                                                                                                   target_registers) };
    }

    TransitionDlContext make_dl_transition_context(tyr::planning::StateView<Kind> source_state,
                                                   tyr::planning::StateView<Kind> target_state,
                                                   runir::kr::dl::semantics::CallArgumentsView arguments,
                                                   runir::kr::dl::semantics::RegisterValuesView source_registers,
                                                   runir::kr::dl::semantics::RegisterValuesView target_registers)
    {
        return TransitionDlContext(std::move(source_state),
                                   std::move(target_state),
                                   m_dl_builder,
                                   m_dl_denotation_repository,
                                   m_dl_workspace,
                                   m_dl_caches,
                                   m_dl_target_caches,
                                   arguments,
                                   source_registers,
                                   target_registers);
    }
};

}  // namespace runir::kr::ps::ext

#endif
