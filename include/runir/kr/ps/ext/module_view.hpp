#ifndef RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_

#include "runir/kr/ps/ext/argument_view.hpp"
#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <concepts>
#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/containers/vector.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::ext::Module>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ext::Module> m_handle;

public:
    View(Index<runir::kr::ps::ext::Module> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    const auto& get_name() const noexcept { return get_data().name; }

    template<runir::kr::dl::CategoryTag Category>
    auto get_arguments() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return make_view(get_data().concept_arguments, *m_context);
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            return make_view(get_data().role_arguments, *m_context);
        else if constexpr (std::same_as<Category, runir::kr::dl::BooleanTag>)
            return make_view(get_data().boolean_arguments, *m_context);
        else if constexpr (std::same_as<Category, runir::kr::dl::NumericalTag>)
            return make_view(get_data().numerical_arguments, *m_context);
    }

    auto get_registers() const noexcept { return make_view(get_data().registers, *m_context); }
    auto get_entry_memory_state() const noexcept { return make_view(get_data().entry_memory_state, *m_context); }
    auto get_memory_states() const noexcept { return make_view(get_data().memory_states, *m_context); }
    auto get_memory_transitions() const noexcept { return make_view(get_data().memory_transitions, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
