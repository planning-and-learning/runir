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
#include <type_traits>
#include <tyr/formalism/declarations.hpp>
#include <tyr/planning/declarations.hpp>
#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

/// An activation: its module, memory state, registers and arguments.
template<>
struct data<runir::kr::ps::ext::ModuleState>
{
    urd::index<runir::kr::ps::ext::ModuleState> index;
    urd::index<runir::kr::ps::ext::Module> mod;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> memory_state;
    urd::index<runir::kr::dl::semantics::RegisterValues> registers;
    urd::index<runir::kr::dl::semantics::CallArguments> arguments;

    data() = default;

    data(urd::index<runir::kr::ps::ext::Module> mod_,
         urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> memory_state_,
         urd::index<runir::kr::dl::semantics::RegisterValues> registers_,
         urd::index<runir::kr::dl::semantics::CallArguments> arguments_) :
        index(),
        mod(mod_),
        memory_state(memory_state_),
        registers(registers_),
        arguments(arguments_)
    {
    }

    static constexpr bool copyable() noexcept
    {
        static_assert(std::is_trivially_copyable_v<data>, "a copyable record is trivially copyable");
        return true;
    }

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"mod", &data::mod>,
                           urd::field<"memory_state", &data::memory_state>,
                           urd::field<"registers", &data::registers>,
                           urd::field<"arguments", &data::arguments>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::index, &data::mod, &data::memory_state, &data::registers, &data::arguments> {}; }
};

/// A frame: the suspended activation (its memory state the one to return to) and its caller.
template<>
struct data<runir::kr::ps::ext::CallStack>
{
    urd::index<runir::kr::ps::ext::CallStack> index;
    urd::index<runir::kr::ps::ext::ModuleState> module_state;
    urd::flat_optional<urd::index<runir::kr::ps::ext::CallStack>> caller;

    data() = default;

    data(urd::index<runir::kr::ps::ext::ModuleState> module_state_, urd::flat_optional<urd::index<runir::kr::ps::ext::CallStack>> caller_) :
        index(),
        module_state(module_state_),
        caller(std::move(caller_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"module_state", &data::module_state>, urd::field<"caller", &data::caller>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::module_state, &data::caller> {}; }
};

/// A program state: its planning state, activation and call stack (its program is its creator's: an expander's, an
/// evaluation environment's).
template<tyr::TaskKind Kind>
struct data<runir::kr::ps::ext::ProgramState<Kind>>
{
    urd::index<runir::kr::ps::ext::ProgramState<Kind>> index;
    urd::index<tyr::planning::State<Kind>> state;
    urd::index<runir::kr::ps::ext::ModuleState> module_state;
    urd::flat_optional<urd::index<runir::kr::ps::ext::CallStack>> call_stack;

    data() = default;

    data(urd::index<tyr::planning::State<Kind>> state_,
         urd::index<runir::kr::ps::ext::ModuleState> module_state_,
         urd::flat_optional<urd::index<runir::kr::ps::ext::CallStack>> call_stack_) :
        index(),
        state(state_),
        module_state(module_state_),
        call_stack(std::move(call_stack_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"state", &data::state>, urd::field<"module_state", &data::module_state>, urd::field<"call_stack", &data::call_stack>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::state, &data::module_state, &data::call_stack> {}; }
};

}  // namespace urd

#endif
