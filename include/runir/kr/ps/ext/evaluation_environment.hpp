#ifndef RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_storage.hpp"
#include "runir/kr/dl/semantics/ext/state_evaluation_context.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/task_context.hpp"

#include <stdexcept>
#include <utility>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
class EvaluationEnvironment
{
private:
    runir::kr::dl::semantics::Builder& m_dl_builder;
    runir::kr::dl::semantics::DenotationRepository& m_dl_denotation_repository;
    ExecutionRepository<Kind>& m_execution_repository;
    runir::kr::dl::semantics::EvaluationStorage<runir::kr::ExtFamilyTag> m_dl_storage;
    runir::kr::dl::semantics::EvaluationStorage<runir::kr::ExtFamilyTag> m_dl_target_storage;
    runir::kr::dl::semantics::DenotationCaches<runir::kr::ExtFamilyTag> m_call_caches;
    ProgramView m_program;

public:
    EvaluationEnvironment(runir::kr::TaskContext<Kind>& task_context, ProgramView program) :
        m_dl_builder(task_context.dl_builder),
        m_dl_denotation_repository(*task_context.dl_denotation_repository),
        m_execution_repository(*task_context.execution_repository),
        m_dl_storage(m_dl_denotation_repository),
        m_dl_target_storage(m_dl_denotation_repository),
        m_program(program)
    {
    }

    auto& get_dl_storage() noexcept { return m_dl_storage; }
    auto& get_dl_target_storage() noexcept { return m_dl_target_storage; }
    auto& get_dl_caches() noexcept { return m_dl_storage.get_caches(); }
    auto& get_dl_target_caches() noexcept { return m_dl_target_storage.get_caches(); }

    void reset_source() noexcept
    {
        m_call_caches.reset_dynamic();
        m_dl_storage.reset_dynamic();
    }

    void reset_target() noexcept { m_dl_target_storage.reset_dynamic(); }

    /// Contexts borrow stored register and argument data. Cache invalidation remains explicit.
    template<ProgramStateViewConcept<Kind> S>
    auto make_dl_context(S state)
    {
        const auto program = state.get_program();
        if (&state.get_context() != &m_execution_repository || &program.get_context() != &m_program.get_context() || program != m_program)
            throw std::invalid_argument("EvaluationEnvironment requires an execution state from the selected task and program.");
        const auto module_state = state.get_module_state();
        return make_dl_context(module_state.get_state(), module_state.get_arguments(), module_state.get_registers());
    }

    template<tyr::planning::StateViewConcept<Kind> S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
    auto make_dl_context(S state, runir::kr::dl::semantics::CallArgumentsView arguments, R registers)
    {
        using StateDlContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, Kind, S, R>;
        return StateDlContext(std::move(state), m_dl_builder, m_dl_storage, arguments, registers);
    }

    /// Call argument roots enter task storage directly; their intermediates use
    /// the ordinary source storage. Argument inputs already belong to this task.
    template<tyr::planning::StateViewConcept<Kind> S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
    auto make_call_context(S state, runir::kr::dl::semantics::CallArgumentsView arguments, R registers)
    {
        if (&arguments.get_context() != &m_dl_denotation_repository)
            throw std::invalid_argument("Call argument evaluation requires arguments from the selected task repository.");
        using StateDlContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, Kind, S, R>;
        return StateDlContext(std::move(state), m_dl_builder, m_call_caches, m_dl_denotation_repository, m_dl_storage, arguments, registers);
    }

    template<tyr::planning::StateViewConcept<Kind> S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
    auto
    make_dl_transition_context(S source_state, S target_state, runir::kr::dl::semantics::CallArgumentsView arguments, R source_registers, R target_registers)
    {
        using TransitionDlContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::ExtFamilyTag, Kind, S, R>;
        return TransitionDlContext(std::move(source_state),
                                   std::move(target_state),
                                   m_dl_builder,
                                   m_dl_storage,
                                   m_dl_target_storage,
                                   arguments,
                                   source_registers,
                                   target_registers);
    }
};

}  // namespace runir::kr::ps::ext

#endif
