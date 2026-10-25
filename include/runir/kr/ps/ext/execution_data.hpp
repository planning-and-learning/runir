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
    Index<runir::kr::ps::ext::Module> module_;
    Index<runir::kr::ps::ext::MemoryState> memory_state;
    Index<runir::kr::dl::semantics::RegisterValues> registers;
    Index<runir::kr::dl::semantics::CallArguments> arguments;

    Data() = default;
    // The referenced indices live in different repositories, hence the separate context parameters.
    Data(Index<tyr::planning::State<Kind>> state_,
         Index<runir::kr::ps::ext::Module> module_index,
         Index<runir::kr::ps::ext::MemoryState> memory_state_,
         Index<runir::kr::dl::semantics::RegisterValues> registers_,
         Index<runir::kr::dl::semantics::CallArguments> arguments_) :
        index(),
        state(std::move(state_)),
        module_(std::move(module_index)),
        memory_state(std::move(memory_state_)),
        registers(std::move(registers_)),
        arguments(std::move(arguments_))
    {
    }
    template<typename S, typename P, typename D>
    Data(::ygg::View<Index<tyr::planning::State<Kind>>, S> state_,
         ::ygg::View<Index<runir::kr::ps::ext::Module>, P> module_index,
         ::ygg::View<Index<runir::kr::ps::ext::MemoryState>, P> memory_state_,
         ::ygg::View<Index<runir::kr::dl::semantics::RegisterValues>, D> registers_,
         ::ygg::View<Index<runir::kr::dl::semantics::CallArguments>, D> arguments_) :
        index(),
        state(),
        module_(),
        memory_state(),
        registers(),
        arguments()
    {
        set(state_, state);
        set(module_index, module_);
        set(memory_state_, memory_state);
        set(registers_, registers);
        set(arguments_, arguments);
    }

    static constexpr auto cista_members() noexcept
    { return ygg::Members<&Data::index, &Data::state, &Data::module_, &Data::memory_state, &Data::registers, &Data::arguments> {}; }
    static constexpr auto identifying_members() noexcept
    { return ygg::Members<&Data::state, &Data::module_, &Data::memory_state, &Data::registers, &Data::arguments> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<>
struct Data<runir::kr::ps::ext::CallStack>
{
    Index<runir::kr::ps::ext::CallStack> index;
    Index<runir::kr::ps::ext::Module> module_;
    Index<runir::kr::ps::ext::MemoryState> return_memory_state;
    Index<runir::kr::dl::semantics::RegisterValues> registers;
    Index<runir::kr::dl::semantics::CallArguments> arguments;
    ::cista::optional<Index<runir::kr::ps::ext::CallStack>> caller;

    Data() = default;
    // The referenced indices live in different repositories, hence the separate context parameters.
    Data(Index<runir::kr::ps::ext::Module> module_index,
         Index<runir::kr::ps::ext::MemoryState> return_memory_state_,
         Index<runir::kr::dl::semantics::RegisterValues> registers_,
         Index<runir::kr::dl::semantics::CallArguments> arguments_,
         ::cista::optional<Index<runir::kr::ps::ext::CallStack>> caller_) :
        index(),
        module_(std::move(module_index)),
        return_memory_state(std::move(return_memory_state_)),
        registers(std::move(registers_)),
        arguments(std::move(arguments_)),
        caller(std::move(caller_))
    {
    }
    template<typename C, typename P, typename D>
    Data(::ygg::View<Index<runir::kr::ps::ext::Module>, P> module_index,
         ::ygg::View<Index<runir::kr::ps::ext::MemoryState>, P> return_memory_state_,
         ::ygg::View<Index<runir::kr::dl::semantics::RegisterValues>, D> registers_,
         ::ygg::View<Index<runir::kr::dl::semantics::CallArguments>, D> arguments_,
         const std::optional<::ygg::View<Index<runir::kr::ps::ext::CallStack>, C>>& caller_) :
        index(),
        module_(),
        return_memory_state(),
        registers(),
        arguments(),
        caller()
    {
        set(module_index, module_);
        set(return_memory_state_, return_memory_state);
        set(registers_, registers);
        set(arguments_, arguments);
        set(caller_, caller);
    }

    static constexpr auto cista_members() noexcept
    { return ygg::Members<&Data::index, &Data::module_, &Data::return_memory_state, &Data::registers, &Data::arguments, &Data::caller> {}; }
    static constexpr auto identifying_members() noexcept
    { return ygg::Members<&Data::module_, &Data::return_memory_state, &Data::registers, &Data::arguments, &Data::caller> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

template<tyr::TaskKind Kind>
struct Data<runir::kr::ps::ext::ProgramState<Kind>>
{
    Index<runir::kr::ps::ext::ProgramState<Kind>> index;
    Index<runir::kr::ps::ext::Program> program;
    Index<runir::kr::ps::ext::ModuleState<Kind>> module_state;
    ::cista::optional<Index<runir::kr::ps::ext::CallStack>> call_stack;

    Data() = default;
    // The referenced indices live in different repositories, hence the separate context parameters.
    Data(Index<runir::kr::ps::ext::Program> program_,
         Index<runir::kr::ps::ext::ModuleState<Kind>> module_state_,
         ::cista::optional<Index<runir::kr::ps::ext::CallStack>> call_stack_) :
        index(),
        program(std::move(program_)),
        module_state(std::move(module_state_)),
        call_stack(std::move(call_stack_))
    {
    }
    template<typename C, typename P>
    Data(::ygg::View<Index<runir::kr::ps::ext::Program>, P> program_,
         ::ygg::View<Index<runir::kr::ps::ext::ModuleState<Kind>>, C> module_state_,
         const std::optional<::ygg::View<Index<runir::kr::ps::ext::CallStack>, C>>& call_stack_) :
        index(),
        program(),
        module_state(),
        call_stack()
    {
        set(program_, program);
        set(module_state_, module_state);
        set(call_stack_, call_stack);
    }

    static constexpr auto cista_members() noexcept { return ygg::Members<&Data::index, &Data::program, &Data::module_state, &Data::call_stack> {}; }
    static constexpr auto identifying_members() noexcept { return ygg::Members<&Data::program, &Data::module_state, &Data::call_stack> {}; }
    void clear() noexcept { ygg::clear_members(*this); }
};

}  // namespace ygg

#endif
