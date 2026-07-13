#ifndef RUNIR_KR_PS_EXT_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_EXT_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/module_program_view.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <concepts>
#include <tuple>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/dependent_false.hpp>

namespace runir::kr::ps::ext
{

using runir::kr::dl::semantics::Registers;

struct CallArguments
{
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::ConceptTag>> concept_arguments;
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::RoleTag>> role_arguments;
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::BooleanTag>> boolean_arguments;
    std::vector<runir::kr::dl::semantics::DenotationView<runir::kr::dl::NumericalTag>> numerical_arguments;

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

    auto identifying_members() const noexcept { return std::tie(concept_arguments, role_arguments, boolean_arguments, numerical_arguments); }
};

struct CallFrame
{
    ModuleView module_;
    MemoryStateView return_memory_state;
    Registers registers;
    CallArguments arguments;

    auto identifying_members() const noexcept { return std::tie(module_, return_memory_state, registers, arguments); }
};

struct CallStack
{
private:
    ModuleView m_module;
    MemoryStateView m_memory_state;
    Registers m_registers;
    CallArguments m_arguments;
    CallArguments m_call_arguments;
    std::vector<CallFrame> m_frames;

public:
    CallStack(ModuleView module, MemoryStateView memory_state, Registers registers, CallArguments arguments = {}, std::vector<CallFrame> frames = {}) noexcept :
        m_module(module),
        m_memory_state(memory_state),
        m_registers(std::move(registers)),
        m_arguments(std::move(arguments)),
        m_call_arguments(),
        m_frames(std::move(frames))
    {
    }

    CallStack(ModuleView module, CallArguments arguments = {}) noexcept : CallStack(module, module.get_entry_memory_state(), Registers {}, std::move(arguments))
    {
    }

    const auto& module() const noexcept { return m_module; }
    const auto& memory_state() const noexcept { return m_memory_state; }
    void set_memory_state(MemoryStateView memory_state) noexcept { m_memory_state = memory_state; }

    auto& registers() noexcept { return m_registers; }
    const auto& registers() const noexcept { return m_registers; }
    auto& arguments() noexcept { return m_arguments; }
    const auto& arguments() const noexcept { return m_arguments; }
    const auto& frames() const noexcept { return m_frames; }
    bool has_caller() const noexcept { return !m_frames.empty(); }

    auto identifying_members() const noexcept { return std::tie(m_module, m_memory_state, m_registers, m_arguments, m_frames); }

    CallArguments& prepare_call_arguments() noexcept
    {
        m_call_arguments.clear();
        return m_call_arguments;
    }

    void enter_module(ModuleView module, MemoryStateView return_memory_state, CallArguments arguments = {})
    {
        m_frames.push_back(CallFrame { m_module, return_memory_state, std::move(m_registers), std::move(m_arguments) });

        m_module = module;
        m_memory_state = module.get_entry_memory_state();
        m_registers = Registers {};
        m_arguments = std::move(arguments);
    }

    bool restore_caller()
    {
        if (m_frames.empty())
            return false;

        auto frame = std::move(m_frames.back());
        m_frames.pop_back();

        m_call_arguments = std::move(m_arguments);
        m_module = frame.module_;
        m_memory_state = frame.return_memory_state;
        m_registers = std::move(frame.registers);
        m_arguments = std::move(frame.arguments);
        return true;
    }
};

template<tyr::planning::TaskKind Kind>
class EvaluationContext
{
private:
    tyr::planning::StateView<Kind> m_state;
    ModuleProgramView m_program;
    CallStack m_call_stack;

public:
    EvaluationContext(tyr::planning::StateView<Kind> state, ModuleProgramView program, ModuleView module, CallArguments arguments = {}) noexcept :
        m_state(std::move(state)),
        m_program(program),
        m_call_stack(module, std::move(arguments))
    {
    }

    EvaluationContext(tyr::planning::StateView<Kind> state,
                      ModuleProgramView program,
                      ModuleView module,
                      MemoryStateView memory_state,
                      Registers registers,
                      CallArguments arguments = {},
                      std::vector<CallFrame> frames = {}) noexcept :
        m_state(std::move(state)),
        m_program(program),
        m_call_stack(module, memory_state, std::move(registers), std::move(arguments), std::move(frames))
    {
    }

    auto& get_state() noexcept { return m_state; }
    const auto& get_state() const noexcept { return m_state; }

    auto get_program() const noexcept { return m_program; }
    auto& get_call_stack() noexcept { return m_call_stack; }
    const auto& get_call_stack() const noexcept { return m_call_stack; }

    auto identifying_members() const noexcept { return std::tie(m_state, m_program, m_call_stack); }
};

}  // namespace runir::kr::ps::ext

#endif
