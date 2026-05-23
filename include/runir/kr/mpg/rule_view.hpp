#ifndef RUNIR_KR_MPG_RULE_VIEW_HPP_
#define RUNIR_KR_MPG_RULE_VIEW_HPP_

#include "runir/kr/mpg/memory_state_view.hpp"
#include "runir/kr/mpg/rule_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/vector.hpp>

namespace tyr
{

template<typename Kind, typename C>
class View<Index<runir::kr::mpg::Rule<Kind>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::mpg::Rule<Kind>> m_handle;

public:
    View(Index<runir::kr::mpg::Rule<Kind>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_source() const noexcept { return View<Index<runir::kr::mpg::MemoryState>, C>(get_data().source, *m_context); }
    auto get_target() const noexcept { return View<Index<runir::kr::mpg::MemoryState>, C>(get_data().target, *m_context); }
    auto get_conditions() const noexcept { return make_view(get_data().conditions, *m_context); }
    auto get_effects() const noexcept { return make_view(get_data().effects, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
