#ifndef RUNIR_KR_PS_EXT_MEMORY_TRANSITION_VIEW_HPP_
#define RUNIR_KR_PS_EXT_MEMORY_TRANSITION_VIEW_HPP_

#include "runir/kr/ps/ext/memory_state_view.hpp"
#include "runir/kr/ps/ext/memory_transition_data.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>
#include <yggdrasil/containers/vector.hpp>

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::ext::MemoryTransition>, C>
{
private:
    const C* m_context;
    Index<runir::kr::ps::ext::MemoryTransition> m_handle;

public:
    View(Index<runir::kr::ps::ext::MemoryTransition> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_source() const noexcept { return make_view(get_data().source, *m_context); }
    auto get_target() const noexcept { return make_view(get_data().target, *m_context); }
    const auto& get_symbol() const noexcept { return get_data().symbol; }
    const auto& get_description() const noexcept { return get_data().description; }
    auto get_rules() const noexcept { return make_view(get_data().rules, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
