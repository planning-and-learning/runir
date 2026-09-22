#ifndef RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/ext/state_evaluation_context.hpp"
#include "runir/kr/ps/ext/dl/transition_evaluation_context.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/task_context.hpp"

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext
{

/// Caller-owned denotation views. Keep this storage alive and unchanged while
/// evaluation contexts borrow its argument spans.
struct EvaluationArguments
{
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::ConceptTag>> concept_arguments;
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::RoleTag>> role_arguments;
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::BooleanTag>> boolean_arguments;
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::NumericalTag>> numerical_arguments;

    EvaluationArguments() = default;

    template<tyr::TaskKind Kind>
    explicit EvaluationArguments(CallArgumentsView<Kind> arguments) :
        concept_arguments(arguments.template get<runir::kr::dl::ConceptTag>()),
        role_arguments(arguments.template get<runir::kr::dl::RoleTag>()),
        boolean_arguments(arguments.template get<runir::kr::dl::BooleanTag>()),
        numerical_arguments(arguments.template get<runir::kr::dl::NumericalTag>())
    {
    }

    runir::kr::dl::semantics::Arguments view() const noexcept { return { concept_arguments, role_arguments, boolean_arguments, numerical_arguments }; }
};

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
    ModuleProgramView m_program;

public:
    EvaluationEnvironment(runir::kr::TaskContext<Kind>& task_context, ModuleProgramView program) :
        m_dl_builder(task_context.dl_builder),
        m_dl_denotation_repository(*task_context.dl_denotation_repository),
        m_execution_repository(*task_context.execution_repository),
        m_dl_workspace(),
        m_dl_caches(),
        m_dl_target_caches(),
        m_do_argument_denotations(),
        m_program(program)
    {
    }

    auto get_program() const noexcept { return m_program; }
    auto& get_dl_repository() noexcept { return m_program.get_context().get_dl_repository(); }
    auto& get_dl_workspace() noexcept { return m_dl_workspace; }
    auto& get_dl_caches() noexcept { return m_dl_caches; }
    auto& get_dl_target_caches() noexcept { return m_dl_target_caches; }

    auto& prepare_do_argument_denotations(size_t size)
    {
        m_do_argument_denotations.clear();
        m_do_argument_denotations.reserve(size);
        return m_do_argument_denotations;
    }

    /// Contexts borrow caller-owned arguments. Cache invalidation remains explicit.
    StateDlContext make_dl_context(ExecutionStateView<Kind> state, const EvaluationArguments& arguments)
    {
        const auto program = state.get_program();
        if (&state.get_context() != &m_execution_repository || &program.get_context() != &m_program.get_context()
            || program.get_index() != m_program.get_index())
            throw std::invalid_argument("EvaluationEnvironment requires an execution state from the selected task and program.");
        const auto stack = state.get_call_stack();
        return make_dl_context(state.get_state(), arguments.view(), make_registers(stack.get_registers()));
    }

    StateDlContext
    make_dl_context(tyr::planning::StateView<Kind> state, runir::kr::dl::semantics::Arguments arguments, runir::kr::dl::semantics::Registers registers)
    {
        return StateDlContext(std::move(state), m_dl_builder, m_dl_denotation_repository, m_dl_workspace, m_dl_caches, arguments, std::move(registers));
    }

    TransitionDlContext make_dl_transition_context(tyr::planning::StateView<Kind> source_state,
                                                   tyr::planning::StateView<Kind> target_state,
                                                   runir::kr::dl::semantics::Arguments arguments,
                                                   runir::kr::dl::semantics::Registers registers)
    {
        return TransitionDlContext(std::move(source_state),
                                   std::move(target_state),
                                   m_dl_builder,
                                   m_dl_denotation_repository,
                                   m_dl_workspace,
                                   m_dl_caches,
                                   m_dl_target_caches,
                                   arguments,
                                   std::move(registers));
    }

    static runir::kr::dl::semantics::Registers make_registers(RegisterValuesView<Kind> registers)
    {
        auto result = runir::kr::dl::semantics::Registers {};
        auto& concepts = result.template get<runir::kr::dl::ConceptTag>();
        auto& roles = result.template get<runir::kr::dl::RoleTag>();
        const auto concept_values = registers.get_concept_values();
        const auto role_values = registers.get_role_values();
        for (size_t i = 0; i < concept_values.size(); ++i)
        {
            if (const auto value = concept_values[i])
                concepts[i] = value.value();
            if (const auto value = role_values[i])
            {
                const auto pair = value.value();
                roles[i] = std::pair(pair.get_first(), pair.get_second());
            }
        }
        return result;
    }
};

}  // namespace runir::kr::ps::ext

#endif
