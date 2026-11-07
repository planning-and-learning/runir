#ifndef RUNIR_KR_PS_EXT_EXECUTION_DATA_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_DATA_HPP_

#include "runir/kr/dl/semantics/call_arguments_data.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"

#include <cista/containers/optional.h>
#include <optional>
#include <tuple>
#include <tyr/formalism/declarations.hpp>
#include <tyr/planning/declarations.hpp>
#include <utility>
#include <urd/elements/flat.hpp>

namespace urd
{

template<tyr::TaskKind Kind>
struct data<runir::kr::ps::ext::ModuleState<Kind>>
{
    urd::index<runir::kr::ps::ext::ModuleState<Kind>> index;
    urd::index<tyr::planning::State<Kind>> state;
    urd::index<runir::kr::ps::ext::Module> module_;
    urd::index<runir::kr::ps::ext::MemoryState> memory_state;
    urd::index<runir::kr::dl::semantics::RegisterValues> registers;
    urd::index<runir::kr::dl::semantics::CallArguments> arguments;

    data() = default;
    // The referenced indices live in different repositories, hence the separate context parameters.
    data(urd::index<tyr::planning::State<Kind>> state_,
         urd::index<runir::kr::ps::ext::Module> module_index,
         urd::index<runir::kr::ps::ext::MemoryState> memory_state_,
         urd::index<runir::kr::dl::semantics::RegisterValues> registers_,
         urd::index<runir::kr::dl::semantics::CallArguments> arguments_) :
        index(),
        state(std::move(state_)),
        module_(module_index),
        memory_state(memory_state_),
        registers(registers_),
        arguments(arguments_)
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::state, &data::module_, &data::memory_state, &data::registers, &data::arguments> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::state, &data::module_, &data::memory_state, &data::registers, &data::arguments> {}; }
};

template<>
struct data<runir::kr::ps::ext::CallStack>
{
    urd::index<runir::kr::ps::ext::CallStack> index;
    urd::index<runir::kr::ps::ext::Module> module_;
    urd::index<runir::kr::ps::ext::MemoryState> return_memory_state;
    urd::index<runir::kr::dl::semantics::RegisterValues> registers;
    urd::index<runir::kr::dl::semantics::CallArguments> arguments;
    urd::flat_optional<urd::index<runir::kr::ps::ext::CallStack>> caller;

    data() = default;
    // The referenced indices live in different repositories, hence the separate context parameters.
    data(urd::index<runir::kr::ps::ext::Module> module_index,
         urd::index<runir::kr::ps::ext::MemoryState> return_memory_state_,
         urd::index<runir::kr::dl::semantics::RegisterValues> registers_,
         urd::index<runir::kr::dl::semantics::CallArguments> arguments_,
         urd::flat_optional<urd::index<runir::kr::ps::ext::CallStack>> caller_) :
        index(),
        module_(module_index),
        return_memory_state(return_memory_state_),
        registers(registers_),
        arguments(arguments_),
        caller(std::move(caller_))
    {
    }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::module_, &data::return_memory_state, &data::registers, &data::arguments, &data::caller> {}; }
    static constexpr auto identifying_members() noexcept
    { return urd::Members<&data::module_, &data::return_memory_state, &data::registers, &data::arguments, &data::caller> {}; }
};

template<tyr::TaskKind Kind>
struct data<runir::kr::ps::ext::ProgramState<Kind>>
{
    urd::index<runir::kr::ps::ext::ProgramState<Kind>> index;
    urd::index<runir::kr::ps::ext::Program> program;
    urd::index<runir::kr::ps::ext::ModuleState<Kind>> module_state;
    urd::flat_optional<urd::index<runir::kr::ps::ext::CallStack>> call_stack;

    data() = default;
    // The referenced indices live in different repositories, hence the separate context parameters.
    data(urd::index<runir::kr::ps::ext::Program> program_,
         urd::index<runir::kr::ps::ext::ModuleState<Kind>> module_state_,
         urd::flat_optional<urd::index<runir::kr::ps::ext::CallStack>> call_stack_) :
        index(),
        program(program_),
        module_state(std::move(module_state_)),
        call_stack(std::move(call_stack_))
    {
    }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::program, &data::module_state, &data::call_stack> {}; }
    static constexpr auto identifying_members() noexcept { return urd::Members<&data::program, &data::module_state, &data::call_stack> {}; }
};

}  // namespace urd

#endif
