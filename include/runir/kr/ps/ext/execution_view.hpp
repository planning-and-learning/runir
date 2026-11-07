#ifndef RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/ps/ext/execution_builder.hpp"
#include "runir/kr/ps/execution_repository.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/ext/program_view.hpp"

#include <optional>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/object_view.hpp>
#include <tyr/planning/state_repository.hpp>
#include <urd/elements/flat.hpp>
#include <yggdrasil/formalism/declarations.hpp>

/// Getters shared by the index, data, and builder views of a module state. The builder
/// view overrides the two that read inline construction data.
template<tyr::TaskKind Kind, typename V>
struct urd::view_mixin<runir::kr::ps::ext::ModuleState<Kind>, V>
{
private:
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_state() const { return get_repository(self().context()).get_state_repository().get_registered_state(self().data().state); }
    auto get_module() const noexcept { return urd::resolve(self().data().module_, get_repository(self().context()).get_program_repository()); }
    auto get_memory_state() const noexcept { return urd::resolve(self().data().memory_state, get_repository(self().context()).get_program_repository()); }
    auto get_registers() const noexcept { return urd::resolve(self().data().registers, get_repository(self().context()).get_denotation_repository()); }
    auto get_arguments() const noexcept { return urd::resolve(self().data().arguments, get_repository(self().context()).get_denotation_repository()); }
};

/// Getters shared by the index and data views of a call stack frame.
template<typename V>
struct urd::view_mixin<runir::kr::ps::ext::CallStack, V>
{
private:
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_module() const noexcept { return urd::resolve(self().data().module_, get_repository(self().context()).get_program_repository()); }
    auto get_return_memory_state() const noexcept { return urd::resolve(self().data().return_memory_state, get_repository(self().context()).get_program_repository()); }
    auto get_registers() const noexcept { return urd::resolve(self().data().registers, get_repository(self().context()).get_denotation_repository()); }
    auto get_arguments() const noexcept { return urd::resolve(self().data().arguments, get_repository(self().context()).get_denotation_repository()); }

    auto get_caller() const { return urd::resolve(self().data().caller, self().context()); }
};

/// Getters shared by the index, data, and builder views of a program state. A builder may move when
/// search buffers grow; create a builder view only while its owner remains in place.
template<tyr::TaskKind Kind, typename V>
struct urd::view_mixin<runir::kr::ps::ext::ProgramState<Kind>, V>
{
private:
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_program() const noexcept { return urd::resolve(self().data().program, get_repository(self().context()).get_program_repository()); }
    auto get_module_state() const noexcept { return urd::resolve(self().data().module_state, self().context()); }
    auto get_state() const { return get_module_state().get_state(); }

    auto get_call_stack() const
    {
        const auto& call_stack = self().data().call_stack;
        return call_stack ? std::optional(urd::resolve(*call_stack, self().context())) : std::nullopt;
    }
};

/// Borrows inline construction data: the state is not yet registered and the registers live in the
/// formalism repository, so these two getters differ from the interned views.
template<tyr::TaskKind Kind, typename V>
struct urd::view_mixin<urd::builder<runir::kr::ps::ext::ModuleState<Kind>>, V> : urd::view_mixin<runir::kr::ps::ext::ModuleState<Kind>, V>
{
private:
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_state() const { return urd::resolve(self().data().state, get_repository(self().context()).get_state_repository()); }
    auto get_registers() const noexcept { return urd::resolve(self().data().registers, get_repository(self().context()).get_formalism_repository()); }
};

namespace runir::kr::ps::ext
{

template<typename S, typename Kind>
concept ModuleStateViewConcept = tyr::TaskKind<Kind> && requires(const std::remove_reference_t<S>& state) {
    { state.get_state() } -> tyr::planning::StateViewConcept<Kind>;
    { state.get_module() } -> std::same_as<ModuleView>;
    { state.get_memory_state() } -> std::same_as<MemoryStateView>;
    { state.get_registers() } -> runir::kr::dl::semantics::RegisterValuesViewConcept;
    { state.get_arguments() } -> std::same_as<runir::kr::dl::semantics::CallArgumentsView>;
    state.data();
    state.context();
};

/// Shared semantic access for indexed, borrowed-data and builder views; no registered identity is required.
template<typename S, typename Kind>
concept ProgramStateViewConcept = tyr::TaskKind<Kind> && requires(const std::remove_reference_t<S>& state) {
    { state.get_state() } -> tyr::planning::StateViewConcept<Kind>;
    { state.get_program() } -> std::same_as<ProgramView>;
    { state.get_module_state() } -> ModuleStateViewConcept<Kind>;
    { state.get_call_stack() } -> std::same_as<std::optional<CallStackView<Kind>>>;
    state.data();
    state.context();
};

}  // namespace runir::kr::ps::ext

#endif
