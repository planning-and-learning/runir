#ifndef RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/ps/execution_repository.hpp"
#include "runir/kr/ps/ext/execution_builder.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"

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
    auto get_state() const
    {
        const auto state = self().template member<&urd::record_of<V>::state>();
        return get_repository(self().context()).get_state_repository().get_registered_state(state.index());
    }
    auto get_module() const noexcept { return self().template member<&urd::record_of<V>::module_>(); }
    auto get_memory_state() const noexcept { return self().template member<&urd::record_of<V>::memory_state>(); }
    auto get_registers() const noexcept { return self().template member<&urd::record_of<V>::registers>(); }
    auto get_arguments() const noexcept { return self().template member<&urd::record_of<V>::arguments>(); }
};

/// Getters shared by the index and data views of a call stack frame.
template<typename V>
struct urd::view_mixin<runir::kr::ps::ext::CallStack, V>
{
private:
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_module() const noexcept { return self().template member<&urd::record_of<V>::module_>(); }
    auto get_return_memory_state() const noexcept
    { return self().template member<&urd::record_of<V>::return_memory_state>(); }
    auto get_registers() const noexcept { return self().template member<&urd::record_of<V>::registers>(); }
    auto get_arguments() const noexcept { return self().template member<&urd::record_of<V>::arguments>(); }

    auto get_caller() const { return self().template member<&urd::record_of<V>::caller>(); }
};

/// Getters shared by the index, data, and builder views of a program state. A builder may move when
/// search buffers grow; create a builder view only while its owner remains in place.
template<tyr::TaskKind Kind, typename V>
struct urd::view_mixin<runir::kr::ps::ext::ProgramState<Kind>, V>
{
private:
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_program() const noexcept { return self().template member<&urd::record_of<V>::program>(); }
    auto get_module_state() const noexcept { return self().template member<&urd::record_of<V>::module_state>(); }
    auto get_state() const { return get_module_state().get_state(); }

    auto get_call_stack() const { return self().template member<&urd::record_of<V>::call_stack>(); }
};

/// A module state being built: its planning state is not registered yet, so it is read as the builder's view.
template<tyr::TaskKind Kind, typename V>
struct urd::view_mixin<urd::builder<runir::kr::ps::ext::ModuleState<Kind>>, V> : urd::view_mixin<runir::kr::ps::ext::ModuleState<Kind>, V>
{
private:
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_state() const { return self().template member<&urd::record_of<V>::state>(); }
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
    state.context();
};

/// Shared semantic access for indexed, borrowed-data and builder views; no registered identity is required.
template<typename S, typename Kind>
concept ProgramStateViewConcept = tyr::TaskKind<Kind> && requires(const std::remove_reference_t<S>& state) {
    { state.get_state() } -> tyr::planning::StateViewConcept<Kind>;
    { state.get_program() } -> std::same_as<ProgramView>;
    { state.get_module_state() } -> ModuleStateViewConcept<Kind>;
    { state.get_call_stack() } -> std::same_as<std::optional<CallStackView<Kind>>>;
    state.context();
};

}  // namespace runir::kr::ps::ext

#endif
