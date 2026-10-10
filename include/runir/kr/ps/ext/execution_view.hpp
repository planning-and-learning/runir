#ifndef RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/ps/ext/execution_builder.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"
#include "runir/kr/ps/ext/program_view.hpp"

#include <optional>
#include <tuple>
#include <type_traits>
#include <tyr/formalism/object_view.hpp>
#include <tyr/planning/state_view.hpp>
#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/pair.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/formalism/declarations.hpp>

namespace ygg
{

/// Getters shared by the index, data, and builder views of a module state. The builder
/// view overrides the two that read inline construction data.
template<tyr::TaskKind Kind, typename V>
class ViewMixin<runir::kr::ps::ext::ModuleState<Kind>, V>
{
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_state() const { return self().get_repository().get_state_repository().get_registered_state(self().get_data().state); }
    auto get_module() const noexcept { return make_view(self().get_data().module_, self().get_repository().get_program_repository()); }
    auto get_memory_state() const noexcept { return make_view(self().get_data().memory_state, self().get_repository().get_program_repository()); }
    auto get_registers() const noexcept { return make_view(self().get_data().registers, self().get_repository().get_denotation_repository()); }
    auto get_arguments() const noexcept { return make_view(self().get_data().arguments, self().get_repository().get_denotation_repository()); }
};

/// Getters shared by the index and data views of a call stack frame.
template<typename V>
class ViewMixin<runir::kr::ps::ext::CallStack, V>
{
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_module() const noexcept { return make_view(self().get_data().module_, self().get_repository().get_program_repository()); }
    auto get_return_memory_state() const noexcept
    {
        return make_view(self().get_data().return_memory_state, self().get_repository().get_program_repository());
    }
    auto get_registers() const noexcept { return make_view(self().get_data().registers, self().get_repository().get_denotation_repository()); }
    auto get_arguments() const noexcept { return make_view(self().get_data().arguments, self().get_repository().get_denotation_repository()); }

    auto get_caller() const
    {
        using CallerView = decltype(make_view(*self().get_data().caller, self().get_context()));
        if (!self().get_data().caller)
            return std::optional<CallerView> {};
        return std::optional<CallerView>(make_view(*self().get_data().caller, self().get_context()));
    }
};

/// Getters shared by the index, data, and builder views of a program state. A builder may move when
/// search buffers grow; create a builder view only while its owner remains in place.
template<tyr::TaskKind Kind, typename V>
class ViewMixin<runir::kr::ps::ext::ProgramState<Kind>, V>
{
    const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    auto get_program() const noexcept { return make_view(self().get_data().program, self().get_repository().get_program_repository()); }
    auto get_module_state() const noexcept { return make_view(self().get_data().module_state, self().get_context()); }
    auto get_state() const { return get_module_state().get_state(); }

    auto get_call_stack() const
    {
        using CallStackView = decltype(make_view(*self().get_data().call_stack, self().get_context()));
        if (!self().get_data().call_stack)
            return std::optional<CallStackView> {};
        return std::optional<CallStackView>(make_view(*self().get_data().call_stack, self().get_context()));
    }
};

/// Borrows inline construction data: the state is not yet registered and the registers live in the
/// formalism repository, so these two getters differ from the interned views.
template<tyr::TaskKind Kind, typename C>
class View<Builder<runir::kr::ps::ext::ModuleState<Kind>>, C> :
    public ygg::BuilderViewBase<runir::kr::ps::ext::ModuleState<Kind>, C>,
    public ViewMixin<runir::kr::ps::ext::ModuleState<Kind>, View<Builder<runir::kr::ps::ext::ModuleState<Kind>>, C>>
{
public:
    using ygg::BuilderViewBase<runir::kr::ps::ext::ModuleState<Kind>, C>::BuilderViewBase;

    auto get_state() const { return make_view(this->get_data().state, *this->get_repository().get_state_repository().get_task()); }
    auto get_registers() const noexcept { return make_view(this->get_data().registers, this->get_repository().get_formalism_repository()); }
};

}  // namespace ygg

namespace runir::kr::ps::ext
{

template<typename S, typename Kind>
concept ModuleStateViewConcept = tyr::TaskKind<Kind> && requires(const std::remove_reference_t<S>& state) {
    { state.get_state() } -> tyr::planning::StateViewConcept<Kind>;
    { state.get_module() } -> std::same_as<ModuleView>;
    { state.get_memory_state() } -> std::same_as<MemoryStateView>;
    { state.get_registers() } -> runir::kr::dl::semantics::RegisterValuesViewConcept;
    { state.get_arguments() } -> std::same_as<runir::kr::dl::semantics::CallArgumentsView>;
    state.get_data();
    state.get_context();
};

/// Shared semantic access for indexed, borrowed-data and builder views; no registered identity is required.
template<typename S, typename Kind>
concept ProgramStateViewConcept = tyr::TaskKind<Kind> && requires(const std::remove_reference_t<S>& state) {
    { state.get_state() } -> tyr::planning::StateViewConcept<Kind>;
    { state.get_program() } -> std::same_as<ProgramView>;
    { state.get_module_state() } -> ModuleStateViewConcept<Kind>;
    { state.get_call_stack() } -> std::same_as<std::optional<CallStackView<Kind>>>;
    state.get_data();
    state.get_context();
};

}  // namespace runir::kr::ps::ext

#endif
