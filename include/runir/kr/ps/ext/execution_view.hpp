#ifndef RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_VIEW_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"

#include <optional>
#include <tuple>
#include <tyr/formalism/object_view.hpp>
#include <yggdrasil/containers/optional.hpp>
#include <yggdrasil/containers/pair.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<tyr::TaskKind Kind, typename C>
class View<Index<runir::kr::ps::ext::ModuleState<Kind>>, C>
{
private:
    Index<runir::kr::ps::ext::ModuleState<Kind>> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::ps::ext::ModuleState<Kind>> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    auto get_state() const { return get_repository(*m_context).get_state_repository().get_registered_state(get_data().state); }
    auto get_module() const noexcept { return make_view(get_data().module, get_repository(*m_context).get_program_repository()); }
    auto get_memory_state() const noexcept { return make_view(get_data().memory_state, get_repository(*m_context).get_program_repository()); }
    auto get_registers() const noexcept { return make_view(get_data().registers, get_repository(*m_context).get_denotation_repository()); }
    auto get_arguments() const noexcept { return make_view(get_data().arguments, get_repository(*m_context).get_denotation_repository()); }

    auto identifying_members() const noexcept { return std::make_tuple(get_handle(), m_context->get_index()); }
};

template<typename C>
class View<Index<runir::kr::ps::ext::CallStack>, C>
{
private:
    Index<runir::kr::ps::ext::CallStack> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::ps::ext::CallStack> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    auto get_module() const noexcept { return make_view(get_data().module, get_repository(*m_context).get_program_repository()); }
    auto get_return_memory_state() const noexcept { return make_view(get_data().return_memory_state, get_repository(*m_context).get_program_repository()); }
    auto get_registers() const noexcept { return make_view(get_data().registers, get_repository(*m_context).get_denotation_repository()); }
    auto get_arguments() const noexcept { return make_view(get_data().arguments, get_repository(*m_context).get_denotation_repository()); }

    auto get_caller() const -> std::optional<View>
    {
        if (!get_data().caller)
            return std::nullopt;
        return make_view(*get_data().caller, *m_context);
    }

    auto identifying_members() const noexcept { return std::make_tuple(get_handle(), m_context->get_index()); }
};

template<tyr::TaskKind Kind, typename C>
class View<Index<runir::kr::ps::ext::ProgramState<Kind>>, C>
{
private:
    Index<runir::kr::ps::ext::ProgramState<Kind>> m_handle;
    const C* m_context;

public:
    View(Index<runir::kr::ps::ext::ProgramState<Kind>> handle, const C& context) noexcept : m_handle(handle), m_context(&context) {}

    const auto& get_data() const { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }
    auto get_index() const noexcept { return m_handle; }

    auto get_state() const { return get_module_state().get_state(); }
    auto get_program() const noexcept { return make_view(get_data().program, get_repository(*m_context).get_program_repository()); }
    auto get_module_state() const noexcept { return make_view(get_data().module_state, *m_context); }

    auto get_call_stack() const -> std::optional<View<Index<runir::kr::ps::ext::CallStack>, C>>
    {
        if (!get_data().call_stack)
            return std::nullopt;
        return make_view(*get_data().call_stack, *m_context);
    }

    auto identifying_members() const noexcept { return std::make_tuple(get_handle(), m_context->get_index()); }
};

}  // namespace ygg

#endif
