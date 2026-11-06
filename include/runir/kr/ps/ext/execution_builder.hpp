#ifndef RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_

#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/planning/state_builder.hpp>
#include <urd/elements/flat.hpp>

namespace urd
{

/// Owned children use urd::builder<T> where available, otherwise urd::data<T>.
/// References to immutable definitions and interned arguments remain repository indices.
/// Views construct semantic accessors with urd::resolve.
template<tyr::TaskKind Kind>
struct builder<runir::kr::ps::ext::ModuleState<Kind>>
{
    urd::builder<tyr::planning::State<Kind>> state;
    urd::index<runir::kr::ps::ext::Module> module_;
    urd::index<runir::kr::ps::ext::MemoryState> memory_state;
    urd::data<runir::kr::dl::semantics::RegisterValues> registers;
    urd::index<runir::kr::dl::semantics::CallArguments> arguments;

    static constexpr auto identifying_members() noexcept
    { return urd::Members<&builder::state, &builder::module_, &builder::memory_state, &builder::registers, &builder::arguments> {}; }
};

/// Owns mutable module values and references an interned caller chain.
/// Views borrow this value and its repository context.
template<tyr::TaskKind Kind>
struct builder<runir::kr::ps::ext::ProgramState<Kind>>
{
    urd::index<runir::kr::ps::ext::Program> program;
    urd::builder<runir::kr::ps::ext::ModuleState<Kind>> module_state;
    ::cista::optional<urd::index<runir::kr::ps::ext::CallStack>> call_stack;

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::program, &builder::module_state, &builder::call_stack> {}; }
};

}  // namespace urd

#endif
