#ifndef RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_

#include "runir/kr/dl/semantics/call_arguments_index.hpp"
#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"
#include "runir/kr/ps/ext/memory_state_index.hpp"
#include "runir/kr/ps/ext/module_index.hpp"
#include "runir/kr/ps/ext/program_index.hpp"

#include <tuple>
#include <tyr/planning/ground/state_builder.hpp>
#include <tyr/planning/lifted/state_builder.hpp>
#include <vector>

namespace ygg
{

/// Owned children use Builder<T> where available, otherwise Data<T>.
/// References to immutable definitions and interned arguments remain repository indices.
/// Views construct semantic accessors with make_view; builders contain no allocation handles.
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

/// Owns saved caller frames in call order; the immediate caller is the last frame.
template<>
struct Builder<runir::kr::ps::ext::CallStack>
{
    struct Frame
    {
        Index<runir::kr::ps::ext::Module> module_;
        Index<runir::kr::ps::ext::MemoryState> return_memory_state;
        Data<runir::kr::dl::semantics::RegisterValues> registers;
        Index<runir::kr::dl::semantics::CallArguments> arguments;

        auto identifying_members() const noexcept { return std::tie(module_, return_memory_state, registers, arguments); }
    };

    std::vector<Frame> frames;

    auto identifying_members() const noexcept { return std::tie(frames); }
};

/// Owns a complete mutable execution value. Views borrow it and its repository context.
/// Allocation and retention belong to execution/search storage, not this representation.
template<tyr::TaskKind Kind>
struct Builder<runir::kr::ps::ext::ProgramState<Kind>>
{
    Index<runir::kr::ps::ext::Program> program;
    Builder<runir::kr::ps::ext::ModuleState<Kind>> module_state;
    Builder<runir::kr::ps::ext::CallStack> call_stack;

    auto identifying_members() const noexcept { return std::tie(program, module_state, call_stack); }
};

}  // namespace ygg

#endif
