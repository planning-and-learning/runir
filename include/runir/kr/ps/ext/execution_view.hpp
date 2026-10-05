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
#include <yggdrasil/formalism/detail/view.hpp>

namespace ygg
{

template<tyr::TaskKind Kind, formalism::SymbolContextFor<runir::kr::ps::ext::ModuleState<Kind>> C>
class View<Index<runir::kr::ps::ext::ModuleState<Kind>>, C> : public formalism::detail::View<Index<runir::kr::ps::ext::ModuleState<Kind>>, C>
{
public:
    View(Index<runir::kr::ps::ext::ModuleState<Kind>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::ps::ext::ModuleState<Kind>>, C>(handle, context)
    {
    }

    auto get_state() const { return get_repository(*this->m_context).get_state_repository().get_registered_state(this->get_data().state); }
    auto get_module() const noexcept { return make_view(this->get_data().module_, get_repository(*this->m_context).get_program_repository()); }
    auto get_memory_state() const noexcept { return make_view(this->get_data().memory_state, get_repository(*this->m_context).get_program_repository()); }
    auto get_registers() const noexcept { return make_view(this->get_data().registers, get_repository(*this->m_context).get_denotation_repository()); }
    auto get_arguments() const noexcept { return make_view(this->get_data().arguments, get_repository(*this->m_context).get_denotation_repository()); }
};

template<formalism::SymbolContextFor<runir::kr::ps::ext::CallStack> C>
class View<Index<runir::kr::ps::ext::CallStack>, C> : public formalism::detail::View<Index<runir::kr::ps::ext::CallStack>, C>
{
public:
    View(Index<runir::kr::ps::ext::CallStack> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::ps::ext::CallStack>, C>(handle, context)
    {
    }

    auto get_module() const noexcept { return make_view(this->get_data().module_, get_repository(*this->m_context).get_program_repository()); }
    auto get_return_memory_state() const noexcept
    {
        return make_view(this->get_data().return_memory_state, get_repository(*this->m_context).get_program_repository());
    }
    auto get_registers() const noexcept { return make_view(this->get_data().registers, get_repository(*this->m_context).get_denotation_repository()); }
    auto get_arguments() const noexcept { return make_view(this->get_data().arguments, get_repository(*this->m_context).get_denotation_repository()); }

    auto get_caller() const -> std::optional<View>
    {
        if (!this->get_data().caller)
            return std::nullopt;
        return make_view(*this->get_data().caller, *this->m_context);
    }
};

template<tyr::TaskKind Kind, formalism::SymbolContextFor<runir::kr::ps::ext::ProgramState<Kind>> C>
class View<Index<runir::kr::ps::ext::ProgramState<Kind>>, C> : public formalism::detail::View<Index<runir::kr::ps::ext::ProgramState<Kind>>, C>
{
public:
    View(Index<runir::kr::ps::ext::ProgramState<Kind>> handle, const C& context) noexcept :
        formalism::detail::View<Index<runir::kr::ps::ext::ProgramState<Kind>>, C>(handle, context)
    {
    }

    auto get_state() const { return get_module_state().get_state(); }
    auto get_program() const noexcept { return make_view(this->get_data().program, get_repository(*this->m_context).get_program_repository()); }
    auto get_module_state() const noexcept { return make_view(this->get_data().module_state, *this->m_context); }

    auto get_call_stack() const -> std::optional<View<Index<runir::kr::ps::ext::CallStack>, C>>
    {
        if (!this->get_data().call_stack)
            return std::nullopt;
        return make_view(*this->get_data().call_stack, *this->m_context);
    }
};

/// Borrows data whose children remain in their repositories.
template<tyr::TaskKind Kind, typename C>
class View<Data<runir::kr::ps::ext::ModuleState<Kind>>, C>
{
private:
    const Data<runir::kr::ps::ext::ModuleState<Kind>>* m_handle;
    const C* m_context;

public:
    View(const Data<runir::kr::ps::ext::ModuleState<Kind>>& handle, const C& context) noexcept : m_handle(&handle), m_context(&context) {}

    const auto& get_data() const noexcept { return *m_handle; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return *m_handle; }

    auto get_state() const { return get_repository(*m_context).get_state_repository().get_registered_state(get_data().state); }
    auto get_module() const noexcept { return make_view(get_data().module_, get_repository(*m_context).get_program_repository()); }
    auto get_memory_state() const noexcept { return make_view(get_data().memory_state, get_repository(*m_context).get_program_repository()); }
    auto get_registers() const noexcept { return make_view(get_data().registers, get_repository(*m_context).get_denotation_repository()); }
    auto get_arguments() const noexcept { return make_view(get_data().arguments, get_repository(*m_context).get_denotation_repository()); }
};

/// Borrows data whose children remain in their repositories.
template<typename C>
class View<Data<runir::kr::ps::ext::CallStack>, C>
{
private:
    const Data<runir::kr::ps::ext::CallStack>* m_handle;
    const C* m_context;

public:
    View(const Data<runir::kr::ps::ext::CallStack>& handle, const C& context) noexcept : m_handle(&handle), m_context(&context) {}

    const auto& get_data() const noexcept { return *m_handle; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return *m_handle; }

    auto get_module() const noexcept { return make_view(get_data().module_, get_repository(*m_context).get_program_repository()); }
    auto get_return_memory_state() const noexcept { return make_view(get_data().return_memory_state, get_repository(*m_context).get_program_repository()); }
    auto get_registers() const noexcept { return make_view(get_data().registers, get_repository(*m_context).get_denotation_repository()); }
    auto get_arguments() const noexcept { return make_view(get_data().arguments, get_repository(*m_context).get_denotation_repository()); }

    auto get_caller() const -> std::optional<View<Index<runir::kr::ps::ext::CallStack>, C>>
    {
        if (!get_data().caller)
            return std::nullopt;
        return make_view(*get_data().caller, *m_context);
    }
};

/// Borrows data whose children remain in their repositories.
template<tyr::TaskKind Kind, typename C>
class View<Data<runir::kr::ps::ext::ProgramState<Kind>>, C>
{
private:
    const Data<runir::kr::ps::ext::ProgramState<Kind>>* m_handle;
    const C* m_context;

public:
    View(const Data<runir::kr::ps::ext::ProgramState<Kind>>& handle, const C& context) noexcept : m_handle(&handle), m_context(&context) {}

    const auto& get_data() const noexcept { return *m_handle; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return *m_handle; }

    auto get_state() const { return get_module_state().get_state(); }
    auto get_program() const noexcept { return make_view(get_data().program, get_repository(*m_context).get_program_repository()); }
    auto get_module_state() const noexcept { return make_view(get_data().module_state, *m_context); }

    auto get_call_stack() const -> std::optional<View<Index<runir::kr::ps::ext::CallStack>, C>>
    {
        if (!get_data().call_stack)
            return std::nullopt;
        return make_view(*get_data().call_stack, *m_context);
    }
};

/// Borrows inline construction data and supplies its repository context.
template<tyr::TaskKind Kind, typename C>
class View<Builder<runir::kr::ps::ext::ModuleState<Kind>>, C>
{
    const Builder<runir::kr::ps::ext::ModuleState<Kind>>* m_handle;
    const C* m_context;

public:
    View(const Builder<runir::kr::ps::ext::ModuleState<Kind>>& handle, const C& context) noexcept : m_handle(&handle), m_context(&context) {}

    const auto& get_data() const noexcept { return *m_handle; }
    const auto& get_handle() const noexcept { return *m_handle; }
    const auto& get_context() const noexcept { return *m_context; }
    auto get_state() const { return make_view(get_data().state, *get_repository(*m_context).get_state_repository().get_task()); }
    auto get_module() const noexcept { return make_view(get_data().module_, get_repository(*m_context).get_program_repository()); }
    auto get_memory_state() const noexcept { return make_view(get_data().memory_state, get_repository(*m_context).get_program_repository()); }
    auto get_registers() const noexcept { return make_view(get_data().registers, get_repository(*m_context).get_formalism_repository()); }
    auto get_arguments() const noexcept { return make_view(get_data().arguments, get_repository(*m_context).get_denotation_repository()); }
};

/// The builder may move when search buffers grow. Create this view only while its owner remains in place.
template<tyr::TaskKind Kind, typename C>
class View<Builder<runir::kr::ps::ext::ProgramState<Kind>>, C>
{
    const Builder<runir::kr::ps::ext::ProgramState<Kind>>* m_handle;
    const C* m_context;

public:
    View(const Builder<runir::kr::ps::ext::ProgramState<Kind>>& handle, const C& context) noexcept : m_handle(&handle), m_context(&context) {}

    const auto& get_data() const noexcept { return *m_handle; }
    const auto& get_handle() const noexcept { return *m_handle; }
    const auto& get_context() const noexcept { return *m_context; }
    auto get_state() const { return get_module_state().get_state(); }
    auto get_program() const noexcept { return make_view(get_data().program, get_repository(*m_context).get_program_repository()); }
    auto get_module_state() const noexcept { return make_view(get_data().module_state, *m_context); }

    auto get_call_stack() const -> std::optional<View<Index<runir::kr::ps::ext::CallStack>, C>>
    {
        if (!get_data().call_stack)
            return std::nullopt;
        return make_view(*get_data().call_stack, *m_context);
    }
};

}  // namespace ygg

namespace runir::kr::ps::ext
{

template<typename S, typename Kind>
concept ModuleStateViewConcept = tyr::TaskKind<Kind> && requires(const S& state) {
    requires tyr::planning::StateViewConcept<std::remove_cvref_t<decltype(state.get_state())>, Kind>;
    { state.get_module() } -> std::same_as<ModuleView>;
    { state.get_memory_state() } -> std::same_as<MemoryStateView>;
    requires runir::kr::dl::semantics::RegisterValuesViewConcept<std::remove_cvref_t<decltype(state.get_registers())>>;
    { state.get_arguments() } -> std::same_as<runir::kr::dl::semantics::CallArgumentsView>;
    state.get_data();
    state.get_context();
};

/// Shared semantic access for indexed, borrowed-data and builder views; no registered identity is required.
template<typename S, typename Kind>
concept ProgramStateViewConcept = tyr::TaskKind<Kind> && requires(const S& state) {
    requires tyr::planning::StateViewConcept<std::remove_cvref_t<decltype(state.get_state())>, Kind>;
    { state.get_program() } -> std::same_as<ProgramView>;
    requires ModuleStateViewConcept<std::remove_cvref_t<decltype(state.get_module_state())>, Kind>;
    { state.get_call_stack() } -> std::same_as<std::optional<CallStackView<Kind>>>;
    state.get_data();
    state.get_context();
};

}  // namespace runir::kr::ps::ext

#endif
