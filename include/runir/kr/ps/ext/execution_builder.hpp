#ifndef RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/planning/state_builder.hpp>
#include <tyr/planning/state_view.hpp>
#include <urd/elements/flat.hpp>

namespace runir::kr::ps::ext
{
/// The planning state a module state is built over: a state builder, which its views read through the task's state
/// repository (urd::resolution below), not through the execution repository the module state is read through.
template<tyr::TaskKind Kind>
struct BuiltState
{
    urd::builder<tyr::planning::State<Kind>> builder;

    static constexpr auto identifying_members() noexcept { return urd::Members<&BuiltState::builder> {}; }
};

/// The register values a module state is built with, which its views read through the task's formalism repository
/// (urd::resolution below), not through the execution repository the module state is read through.
struct BuiltRegisters
{
    urd::data<runir::kr::dl::semantics::RegisterValues> values;

    static constexpr auto identifying_members() noexcept { return urd::Members<&BuiltRegisters::values> {}; }
};
}  // namespace runir::kr::ps::ext

/// A built state read through an execution repository: its builder's view through the repository's state repository.
template<tyr::TaskKind Kind>
struct urd::resolution<runir::kr::ps::ext::BuiltState<Kind>>
{
    template<typename C>
    static tyr::planning::BuilderStateView<Kind> of(const runir::kr::ps::ext::BuiltState<Kind>& member, const C& context) noexcept
    { return urd::resolve(member.builder, context.get_state_repository()); }

    template<typename C>
    static void of(const runir::kr::ps::ext::BuiltState<Kind>&& member, const C& context) = delete;
};

/// Built registers read through an execution repository: their view through the repository's formalism repository.
template<>
struct urd::resolution<runir::kr::ps::ext::BuiltRegisters>
{
    template<typename C>
    static runir::kr::dl::semantics::BorrowedRegisterValuesView of(const runir::kr::ps::ext::BuiltRegisters& member, const C& context) noexcept
    { return urd::resolve(member.values, context.get_formalism_repository()); }

    template<typename C>
    static void of(const runir::kr::ps::ext::BuiltRegisters&& member, const C& context) = delete;
};

namespace urd
{

/// Owned children use urd::builder<T> where available, otherwise urd::data<T>.
/// References to immutable definitions and interned arguments remain repository indices.
/// Views construct semantic accessors with urd::resolve.
template<tyr::TaskKind Kind>
struct builder<runir::kr::ps::ext::ModuleState<Kind>>
{
    runir::kr::ps::ext::BuiltState<Kind> state;
    urd::index<runir::kr::ps::ext::Module> mod;
    urd::index<runir::kr::ps::MemoryState<runir::kr::ExtFamilyTag>> memory_state;
    runir::kr::ps::ext::BuiltRegisters registers;
    urd::index<runir::kr::dl::semantics::CallArguments> arguments;

    static constexpr auto identifying_members() noexcept
    { return urd::Members<&builder::state, &builder::mod, &builder::memory_state, &builder::registers, &builder::arguments> {}; }
};

/// Owns mutable module values and references an interned caller chain.
/// Views borrow this value and its repository context.
template<tyr::TaskKind Kind>
struct builder<runir::kr::ps::ext::ProgramState<Kind>>
{
    urd::index<runir::kr::ps::ext::Program> program;
    urd::builder<runir::kr::ps::ext::ModuleState<Kind>> module_state;
    urd::flat_optional<urd::index<runir::kr::ps::ext::CallStack>> call_stack;

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::program, &builder::module_state, &builder::call_stack> {}; }
};

}  // namespace urd

#endif
