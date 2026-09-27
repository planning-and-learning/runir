#ifndef RUNIR_KR_PS_ICP_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_ICP_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/ext/state_evaluation_context.hpp"
#include "runir/kr/ps/icp/dl/transition_evaluation_context.hpp"
#include "runir/kr/ps/icp/execution_view.hpp"
#include "runir/kr/task_context.hpp"

namespace runir::kr::ps::icp
{

template<tyr::TaskKind Kind>
class EvaluationEnvironment
{
    using StateContext = runir::kr::dl::semantics::StateEvaluationContext<ExtFamilyTag, Kind>;
    using TransitionContext = runir::kr::ps::dl::TransitionEvaluationContext<IcpFamilyTag, Kind>;
    TaskContext<Kind>& m_task;
    ProgramView m_program;
    runir::kr::dl::semantics::EvaluationWorkspace m_workspace;
    runir::kr::dl::semantics::DenotationCaches<ExtFamilyTag> m_source_caches, m_target_caches;
    runir::kr::dl::semantics::CallArgumentsView m_arguments;

    static auto empty_arguments(TaskContext<Kind>& task)
    {
        auto data = checkout<runir::kr::dl::semantics::CallArguments>(task.dl_builder);
        return get_or_create(*task.dl_denotation_repository, *data).first;
    }

public:
    EvaluationEnvironment(TaskContext<Kind>& task, ProgramView program) :
        m_task(task),
        m_program(program),
        m_source_caches(*task.dl_denotation_repository),
        m_target_caches(*task.dl_denotation_repository),
        m_arguments(empty_arguments(task))
    {
    }

    auto get_program() const noexcept { return m_program; }
    auto& get_dl_repository() noexcept { return m_program.get_context().get_dl_repository(); }
    auto& get_dl_workspace() noexcept { return m_workspace; }
    auto& get_dl_caches() noexcept { return m_source_caches; }
    auto& get_dl_target_caches() noexcept { return m_target_caches; }

    /// Contexts borrow stored data; callers clear dynamic caches before evaluating a new configuration.
    StateContext make_dl_context(ProgramStateView<Kind> state)
    {
        if (&state.get_context() != m_task.icp_execution_repository.get() || state.get_program() != m_program)
            throw std::invalid_argument("ICP evaluation requires a state from the selected task and program.");
        return make_dl_context(state.get_state(), state.get_registers());
    }
    StateContext make_dl_context(tyr::planning::StateView<Kind> state, runir::kr::dl::semantics::RegisterValuesView registers)
    {
        return StateContext(state, m_task.dl_builder, *m_task.dl_denotation_repository, m_workspace, m_source_caches, m_arguments, registers);
    }
    TransitionContext make_dl_transition_context(tyr::planning::StateView<Kind> source,
                                                 tyr::planning::StateView<Kind> target,
                                                 runir::kr::dl::semantics::RegisterValuesView source_registers,
                                                 runir::kr::dl::semantics::RegisterValuesView target_registers)
    {
        return TransitionContext(source,
                                 target,
                                 m_task.dl_builder,
                                 *m_task.dl_denotation_repository,
                                 m_workspace,
                                 m_source_caches,
                                 m_target_caches,
                                 m_arguments,
                                 source_registers,
                                 target_registers);
    }
};

}

#endif
