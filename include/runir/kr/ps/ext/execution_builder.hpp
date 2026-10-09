#ifndef RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_

#include "runir/kr/dl/semantics/call_arguments_index.hpp"
#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"
#include "runir/kr/ps/ext/execution_index.hpp"
#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/module_index.hpp"
#include "runir/kr/ps/ext/program_index.hpp"

#include <cista/containers/optional.h>
#include <tuple>
#include <tyr/planning/state_builder.hpp>
#include <yggdrasil/serialization/cista_equal_to.hpp>
#include <yggdrasil/serialization/cista_hash.hpp>

namespace ygg
{

/// Owned children use Builder<T> where available, otherwise Data<T>.
/// References to immutable definitions and interned arguments remain repository indices.
/// Views construct semantic accessors with make_view.
template<tyr::TaskKind Kind>
struct Builder<runir::kr::ps::ext::ModuleState<Kind>>
{
    Builder<tyr::planning::State<Kind>> state;
    Index<runir::kr::ps::ext::Module> module_;
    Index<runir::kr::ps::ext::MemoryState> memory_state;
    Data<runir::kr::dl::semantics::RegisterValues> registers;
    Index<runir::kr::dl::semantics::CallArguments> arguments;

    auto identifying_members() const noexcept { return std::tie(state, module_, memory_state, registers, arguments); }
};

/// Owns mutable module values and references an interned caller chain.
/// Views borrow this value and its repository context.
template<tyr::TaskKind Kind>
struct Builder<runir::kr::ps::ext::ProgramState<Kind>>
{
    Index<runir::kr::ps::ext::Program> program;
    Builder<runir::kr::ps::ext::ModuleState<Kind>> module_state;
    ::cista::optional<Index<runir::kr::ps::ext::CallStack>> call_stack;

    auto identifying_members() const noexcept { return std::tie(program, module_state, call_stack); }
};

}  // namespace ygg

#endif
