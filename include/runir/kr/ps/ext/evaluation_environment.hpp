#ifndef RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/ext/state_evaluation_context.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/task_context.hpp"

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
class EvaluationEnvironment
{
private:
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

    auto& get_dl_caches() noexcept { return m_dl_caches; }
    auto& get_dl_target_caches() noexcept { return m_dl_target_caches; }

    auto& prepare_do_argument_denotations()
    {
        m_do_argument_denotations.clear();
        return m_do_argument_denotations;
    }

    /// Contexts borrow stored register and argument data. Cache invalidation remains explicit.
    template<ProgramStateViewConcept<Kind> S>
    auto make_dl_context(S state)
    {
        const auto program = state.get_program();
        if (&state.get_context() != &m_execution_repository || &program.get_context() != &m_program.get_context()
            || program.get_index() != m_program.get_index())
            throw std::invalid_argument("EvaluationEnvironment requires an execution state from the selected task and program.");
        const auto module_state = state.get_module_state();
        return make_dl_context(module_state.get_state(), module_state.get_arguments(), module_state.get_registers());
    }

    template<tyr::planning::StateViewConcept<Kind> S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
    auto make_dl_context(S state, runir::kr::dl::semantics::CallArgumentsView arguments, R registers)
    {
        using StateDlContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, Kind, S, R>;
        return StateDlContext(std::move(state), m_dl_builder, m_dl_denotation_repository, m_dl_workspace, m_dl_caches, arguments, registers);
    }

    template<tyr::planning::StateViewConcept<Kind> S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
    auto
    make_dl_transition_context(S source_state, S target_state, runir::kr::dl::semantics::CallArgumentsView arguments, R source_registers, R target_registers)
    {
        using TransitionDlContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::ExtFamilyTag, Kind, S, R>;
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
