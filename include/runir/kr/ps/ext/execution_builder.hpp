#ifndef RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_BUILDER_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/register_values_data.hpp"
#include "runir/kr/ps/ext/execution_declarations.hpp"
#include "runir/kr/ps/ext/program_view.hpp"

#include <optional>
#include <tuple>
#include <tyr/planning/state_builder.hpp>
#include <utility>
#include <yggdrasil/containers/shared_object_pool.hpp>
#include <yggdrasil/semantics/containers/dynamic_bitset_equal_to.hpp>
#include <yggdrasil/semantics/containers/dynamic_bitset_hash.hpp>

namespace ygg
{

/// Owns per-state execution values. Control-only transitions share the planning builder.
template<tyr::TaskKind Kind>
struct Builder<runir::kr::ps::ext::ModuleState<Kind>>
{
    SharedObjectPoolPtr<Builder<tyr::planning::State<Kind>>> state;
    // Interned definitions and arguments are borrowed; unused pool slots have no views.
    std::optional<runir::kr::ps::ext::ModuleView> module_;
    std::optional<runir::kr::ps::ext::MemoryStateView> memory_state;
    Data<runir::kr::dl::semantics::RegisterValues> registers;
    std::optional<runir::kr::dl::semantics::CallArgumentsView> arguments;

    void on_pool_release() noexcept
    {
        state = {};
        arguments.reset();
        module_.reset();
        memory_state.reset();
    }

    auto identifying_members() const noexcept
    {
        return std::tie(state->template get_atoms<tyr::formalism::FluentTag>(), state->get_numeric_variables(), module_, memory_state, registers, arguments);
    }
};

template<>
struct Builder<runir::kr::ps::ext::CallStack>
{
    std::optional<runir::kr::ps::ext::ModuleView> module_;
    std::optional<runir::kr::ps::ext::MemoryStateView> return_memory_state;
    Data<runir::kr::dl::semantics::RegisterValues> registers;
    std::optional<runir::kr::dl::semantics::CallArgumentsView> arguments;
    SharedObjectPoolPtr<Builder> caller;

    void on_pool_release() noexcept
    {
        arguments.reset();
        module_.reset();
        return_memory_state.reset();
        auto tail = std::move(caller);
        while (tail && tail.ref_count() == 1)
        {
            auto next = std::move(tail->caller);
            tail = {};
            tail = std::move(next);
        }
    }

    auto identifying_members() const noexcept
    {
        return std::tuple_cat(std::tie(module_, return_memory_state, registers, arguments),
                              std::make_tuple(caller ? std::optional(std::cref(*caller)) : std::nullopt));
    }
};

/// Retained by paths, pending steps and memo entries. Views borrow this movable owner only during a call.
/// Checked-out pool handles must be released before their execution storage is destroyed.
template<tyr::TaskKind Kind>
struct Builder<runir::kr::ps::ext::ProgramState<Kind>>
{
    runir::kr::ps::ext::ProgramView program;
    SharedObjectPoolPtr<Builder<runir::kr::ps::ext::ModuleState<Kind>>> module_state;
    SharedObjectPoolPtr<Builder<runir::kr::ps::ext::CallStack>> call_stack;

    Builder(runir::kr::ps::ext::ProgramView program_,
            SharedObjectPoolPtr<Builder<runir::kr::ps::ext::ModuleState<Kind>>> module_state_,
            SharedObjectPoolPtr<Builder<runir::kr::ps::ext::CallStack>> call_stack_) :
        program(program_),
        module_state(std::move(module_state_)),
        call_stack(std::move(call_stack_))
    {
    }

    auto identifying_members() const noexcept
    {
        return std::tuple_cat(std::tie(program, *module_state), std::make_tuple(call_stack ? std::optional(std::cref(*call_stack)) : std::nullopt));
    }
};

}  // namespace ygg

#endif
