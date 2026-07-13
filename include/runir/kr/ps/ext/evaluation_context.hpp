#ifndef RUNIR_KR_PS_EXT_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"

#include <concepts>
#include <cstddef>
#include <optional>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/dependent_false.hpp>
#include <yggdrasil/core/types_utils.hpp>

namespace runir::kr::ps::ext
{

using runir::kr::dl::semantics::Registers;

/// Mutable argument storage used only while evaluating one execution state.
struct EvaluationArguments
{
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::ConceptTag>> concept_arguments;
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::RoleTag>> role_arguments;
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::BooleanTag>> boolean_arguments;
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::NumericalTag>> numerical_arguments;

    EvaluationArguments() = default;

    template<tyr::planning::TaskKind Kind>
    explicit EvaluationArguments(CallArgumentsView<Kind> arguments) :
        concept_arguments(arguments.template get<runir::kr::dl::ConceptTag>()),
        role_arguments(arguments.template get<runir::kr::dl::RoleTag>()),
        boolean_arguments(arguments.template get<runir::kr::dl::BooleanTag>()),
        numerical_arguments(arguments.template get<runir::kr::dl::NumericalTag>())
    {
    }

    template<runir::kr::dl::CategoryTag Category>
    auto& get() noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return role_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return boolean_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return numerical_arguments;
        else
            static_assert(ygg::dependent_false<Category>::value, "unhandled DL argument category");
    }

    template<runir::kr::dl::CategoryTag Category>
    const auto& get() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return concept_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return role_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return boolean_arguments;
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return numerical_arguments;
        else
            static_assert(ygg::dependent_false<Category>::value, "unhandled DL argument category");
    }

    void clear() noexcept
    {
        concept_arguments.clear();
        role_arguments.clear();
        boolean_arguments.clear();
        numerical_arguments.clear();
    }

    void write(ygg::Data<CallArguments>& data) const
    {
        for (const auto argument : concept_arguments)
            data.concept_arguments.push_back(argument.get_index());
        for (const auto argument : role_arguments)
            data.role_arguments.push_back(argument.get_index());
        for (const auto argument : boolean_arguments)
            data.boolean_arguments.push_back(argument.get_index());
        for (const auto argument : numerical_arguments)
            data.numerical_arguments.push_back(argument.get_index());
    }
};

/// Mutable materialization of one persistent call-stack node.
template<tyr::planning::TaskKind Kind>
class EvaluationCallStack
{
private:
    ExecutionRepositoryPtr<Kind> m_repository;
    ModuleView m_module;
    MemoryStateView m_memory_state;
    Registers m_registers;
    EvaluationArguments m_arguments;
    EvaluationArguments m_call_arguments;
    std::optional<CallStackView<Kind>> m_caller;

    auto intern_registers()
    {
        auto data = m_repository->template get_builder<RegisterValues>();
        const auto& concepts = m_registers.template get<runir::kr::dl::ConceptTag>();
        const auto& roles = m_registers.template get<runir::kr::dl::RoleTag>();
        for (size_t i = 0; i < concepts.size(); ++i)
        {
            if (concepts[i])
                data->concept_values[i] = concepts[i]->get_index();
            if (roles[i])
                data->role_values[i] = std::pair(roles[i]->first.get_index(), roles[i]->second.get_index());
        }
        auto [index, created] = m_repository->get_or_create(*data);
        static_cast<void>(created);
        return index;
    }

    auto intern_arguments()
    {
        auto data = m_repository->template get_builder<CallArguments>();
        m_arguments.write(*data);
        auto [index, created] = m_repository->get_or_create(*data);
        static_cast<void>(created);
        return index;
    }

public:
    EvaluationCallStack(ExecutionRepositoryPtr<Kind> repository, ModuleView module, EvaluationArguments arguments = {}) noexcept :
        m_repository(std::move(repository)),
        m_module(module),
        m_memory_state(module.get_entry_memory_state()),
        m_registers(),
        m_arguments(std::move(arguments)),
        m_call_arguments(),
        m_caller()
    {
    }

    explicit EvaluationCallStack(CallStackView<Kind> call_stack) :
        m_repository(call_stack.get_context()),
        m_module(call_stack.get_module()),
        m_memory_state(call_stack.get_memory_state()),
        m_registers { call_stack.get_registers().get_concept_values(), call_stack.get_registers().get_role_values() },
        m_arguments(call_stack.get_arguments()),
        m_call_arguments(),
        m_caller(call_stack.get_caller())
    {
    }

    const auto& module() const noexcept { return m_module; }
    const auto& memory_state() const noexcept { return m_memory_state; }
    void set_memory_state(MemoryStateView memory_state) noexcept { m_memory_state = memory_state; }

    auto& registers() noexcept { return m_registers; }
    const auto& registers() const noexcept { return m_registers; }
    auto& arguments() noexcept { return m_arguments; }
    const auto& arguments() const noexcept { return m_arguments; }
    bool has_caller() const noexcept { return m_caller.has_value(); }

    EvaluationArguments& prepare_call_arguments() noexcept
    {
        m_call_arguments.clear();
        return m_call_arguments;
    }

    CallStackView<Kind> intern(MemoryStateView memory_state)
    {
        auto data = m_repository->template get_builder<CallStack>();
        data->module = m_module.get_index();
        data->memory_state = memory_state.get_index();
        data->registers = intern_registers();
        data->arguments = intern_arguments();
        ygg::set(m_caller, data->caller);
        auto [index, created] = m_repository->get_or_create(*data);
        static_cast<void>(created);
        return ygg::make_view(index, m_repository);
    }

    CallStackView<Kind> intern() { return intern(m_memory_state); }

    void enter_module(ModuleView module, MemoryStateView return_memory_state, EvaluationArguments arguments = {})
    {
        m_caller = intern(return_memory_state);
        m_module = module;
        m_memory_state = module.get_entry_memory_state();
        m_registers = Registers {};
        m_arguments = std::move(arguments);
    }

    bool restore_caller()
    {
        if (!m_caller)
            return false;

        const auto caller = *m_caller;
        m_module = caller.get_module();
        m_memory_state = caller.get_memory_state();
        const auto registers = caller.get_registers();
        m_registers = Registers { registers.get_concept_values(), registers.get_role_values() };
        m_arguments = EvaluationArguments(caller.get_arguments());
        m_caller = caller.get_caller();
        return true;
    }
};

/// Short-lived mutable evaluator; canonical execution states live in ExecutionRepository.
template<tyr::planning::TaskKind Kind>
class EvaluationContext
{
private:
    ExecutionRepositoryPtr<Kind> m_repository;
    tyr::planning::StateView<Kind> m_state;
    EvaluationCallStack<Kind> m_call_stack;

public:
    EvaluationContext(ExecutionRepositoryPtr<Kind> repository,
                      tyr::planning::StateView<Kind> state,
                      ModuleView module,
                      EvaluationArguments arguments = {}) noexcept :
        m_repository(repository),
        m_state(std::move(state)),
        m_call_stack(std::move(repository), module, std::move(arguments))
    {
    }

    explicit EvaluationContext(ExecutionStateView<Kind> state) :
        m_repository(state.get_context()),
        m_state(state.get_state()),
        m_call_stack(state.get_call_stack())
    {
    }

    auto& get_state() noexcept { return m_state; }
    const auto& get_state() const noexcept { return m_state; }

    auto get_program() const noexcept { return m_repository->get_program(); }
    auto& get_call_stack() noexcept { return m_call_stack; }
    const auto& get_call_stack() const noexcept { return m_call_stack; }
    const auto& get_repository() const noexcept { return m_repository; }

    ExecutionStateView<Kind> intern(ExecutionPhase phase)
    {
        const auto call_stack = m_call_stack.intern();
        auto data = m_repository->template get_builder<ExecutionState<Kind>>();
        data->state = m_state.get_index();
        data->call_stack = call_stack.get_index();
        data->phase = phase;
        auto [index, created] = m_repository->get_or_create(*data);
        static_cast<void>(created);
        return ygg::make_view(index, m_repository);
    }
};

}  // namespace runir::kr::ps::ext

#endif
