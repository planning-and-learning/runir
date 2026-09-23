#ifndef RUNIR_KR_PS_EXT_EXECUTION_DATA_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_DATA_HPP_

#include "runir/kr/dl/semantics/call_arguments_data.hpp"
#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/ps/ext/execution_index.hpp"
#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/module_index.hpp"
#include "runir/kr/ps/ext/program_index.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/formalism/object_index.hpp>
#include <tyr/planning/state_index.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/serialization/cista_equal_to.hpp>
#include <yggdrasil/serialization/cista_hash.hpp>

namespace ygg
{

template<tyr::TaskKind Kind>
struct Data<runir::kr::ps::ext::ModuleState<Kind>>
{
    Index<runir::kr::ps::ext::ModuleState<Kind>> index;
    Index<tyr::planning::State<Kind>> state;
    Index<runir::kr::ps::ext::Module> module;
    Index<runir::kr::ps::ext::MemoryState> memory_state;
    Index<runir::kr::dl::semantics::RegisterValues> registers;
    Index<runir::kr::dl::semantics::CallArguments> arguments;

    Data() = default;
    Data(Index<tyr::planning::State<Kind>> state_,
         Index<runir::kr::ps::ext::Module> module_,
         Index<runir::kr::ps::ext::MemoryState> memory_state_,
         Index<runir::kr::dl::semantics::RegisterValues> registers_,
         Index<runir::kr::dl::semantics::CallArguments> arguments_) noexcept :
        state(state_),
        module(module_),
        memory_state(memory_state_),
        registers(registers_),
        arguments(arguments_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(state);
        ygg::clear(module);
        ygg::clear(memory_state);
        ygg::clear(registers);
        ygg::clear(arguments);
    }

    auto cista_members() const noexcept { return std::tie(index, state, module, memory_state, registers, arguments); }
    auto identifying_members() const noexcept { return std::tie(state, module, memory_state, registers, arguments); }
};

template<>
struct Data<runir::kr::ps::ext::CallStack>
{
    Index<runir::kr::ps::ext::CallStack> index;
    Index<runir::kr::ps::ext::Module> module;
    Index<runir::kr::ps::ext::MemoryState> return_memory_state;
    Index<runir::kr::dl::semantics::RegisterValues> registers;
    Index<runir::kr::dl::semantics::CallArguments> arguments;
    ::cista::optional<Index<runir::kr::ps::ext::CallStack>> caller;

    Data() = default;
    Data(Index<runir::kr::ps::ext::Module> module_,
         Index<runir::kr::ps::ext::MemoryState> return_memory_state_,
         Index<runir::kr::dl::semantics::RegisterValues> registers_,
         Index<runir::kr::dl::semantics::CallArguments> arguments_,
         ::cista::optional<Index<runir::kr::ps::ext::CallStack>> caller_ = {}) noexcept :
        module(module_),
        return_memory_state(return_memory_state_),
        registers(registers_),
        arguments(arguments_),
        caller(caller_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(module);
        ygg::clear(return_memory_state);
        ygg::clear(registers);
        ygg::clear(arguments);
        ygg::clear(caller);
    }

    auto cista_members() const noexcept { return std::tie(index, module, return_memory_state, registers, arguments, caller); }
    auto identifying_members() const noexcept { return std::tie(module, return_memory_state, registers, arguments, caller); }
};

template<tyr::TaskKind Kind>
struct Data<runir::kr::ps::ext::ProgramState<Kind>>
{
    Index<runir::kr::ps::ext::ProgramState<Kind>> index;
    Index<runir::kr::ps::ext::Program> program;
    Index<runir::kr::ps::ext::ModuleState<Kind>> module_state;
    ::cista::optional<Index<runir::kr::ps::ext::CallStack>> call_stack;

    Data() = default;
    Data(Index<runir::kr::ps::ext::Program> program_,
         Index<runir::kr::ps::ext::ModuleState<Kind>> module_state_,
         ::cista::optional<Index<runir::kr::ps::ext::CallStack>> call_stack_ = {}) noexcept :
        program(program_),
        module_state(module_state_),
        call_stack(call_stack_)
    {
    }

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(program);
        ygg::clear(module_state);
        ygg::clear(call_stack);
    }

    auto cista_members() const noexcept { return std::tie(index, program, module_state, call_stack); }
    auto identifying_members() const noexcept { return std::tie(program, module_state, call_stack); }
};

}  // namespace ygg

#endif
