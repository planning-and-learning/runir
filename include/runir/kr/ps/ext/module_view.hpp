#ifndef RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MODULE_VIEW_HPP_

#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/module_data.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <optional>
#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/vector.hpp>

namespace tyr
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
    auto get_registers() const noexcept { return make_view(get_data().registers, *m_context); }
    auto get_entry_memory_state() const noexcept { return View<Index<runir::kr::ps::ext::MemoryState>, C>(get_data().entry_memory_state, *m_context); }
    auto get_memory_states() const noexcept { return make_view(get_data().memory_states, *m_context); }
    const auto& get_memory_transitions() const noexcept { return get_data().memory_transitions; }
    auto get_rules(Index<runir::kr::ps::ext::MemoryState> source, Index<runir::kr::ps::ext::MemoryState> target) const
    {
        using RuleListView = View<IndexList<runir::kr::ps::ext::RuleVariant>, C>;
        for (const auto& transition : get_data().memory_transitions)
        {
            if (transition.source == source && transition.target == target)
                return std::optional<RuleListView>(make_view(transition.rules, *m_context));
        }
        return std::optional<RuleListView>();
    }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
