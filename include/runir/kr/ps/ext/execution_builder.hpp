#ifndef RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/ps/ext/control_view.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/memory_state_view.hpp"

#include <optional>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/states/state_view.hpp>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of ModuleState: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::ext::ModuleState, C>
{
    urd::key<runir::kr::ps::ext::ModuleState> index;
    urd::built_t<urd::index<runir::kr::ps::ext::Module>, ::runir::kr::ps::ext::Repository> mod;
    urd::built_t<urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>>, ::runir::kr::ps::ext::Repository> memory_state;
    urd::built_t<urd::index<runir::kr::dl::semantics::RegisterValues>, ::runir::kr::dl::semantics::DenotationRepository> registers;
    urd::built_t<urd::index<runir::kr::dl::semantics::CallArguments>, ::runir::kr::dl::semantics::DenotationRepository> arguments;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"mod", &builder::mod>,
                           urd::field<"memory_state", &builder::memory_state>,
                           urd::field<"registers", &builder::registers>,
                           urd::field<"arguments", &builder::arguments>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of CallStack: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::ps::ext::CallStack, C>
{
    urd::key<runir::kr::ps::ext::CallStack> index;
    urd::built_t<urd::index<runir::kr::ps::ext::ModuleState>, C> module_state;
    urd::built_t<urd::flat_optional<urd::index<runir::kr::ps::ext::CallStack>>, C> caller;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"module_state", &builder::module_state>, urd::field<"caller", &builder::caller>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of ProgramState: its record's members, each in its built form (urd::built_t).
template<tyr::TaskKind Kind, typename C>
struct builder<runir::kr::ps::ext::ProgramState<Kind>, C>
{
    urd::key<runir::kr::ps::ext::ProgramState<Kind>> index;
    urd::built_t<urd::index<tyr::planning::State<Kind>>, ::tyr::planning::StateRepository<Kind>> state;
    urd::built_t<urd::index<runir::kr::ps::ext::ModuleState>, ::runir::kr::ps::ext::ControlRepository> module_state;
    urd::built_t<urd::flat_optional<urd::index<runir::kr::ps::ext::CallStack>>, ::runir::kr::ps::ext::ControlRepository> call_stack;

    static constexpr auto fields() noexcept
    {
        return urd::
            Fields<urd::field<"state", &builder::state>, urd::field<"module_state", &builder::module_state>, urd::field<"call_stack", &builder::call_stack>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
