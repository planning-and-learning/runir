#ifndef RUNIR_KR_MPG_MODULE_VIEW_HPP_
#define RUNIR_KR_MPG_MODULE_VIEW_HPP_

#include "runir/kr/mpg/automata_state_view.hpp"
#include "runir/kr/mpg/dfa_view.hpp"
#include "runir/kr/mpg/module_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/vector.hpp>

namespace tyr
{

template<typename C>
class View<Index<runir::kr::mpg::Module>, C>
{
private:
    const C* m_context;
    Index<runir::kr::mpg::Module> m_handle;

public:
    View(Index<runir::kr::mpg::Module> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    const auto& get_name() const noexcept { return get_data().name; }
    auto get_dfa() const noexcept { return View<Index<runir::kr::mpg::DFA>, C>(get_data().dfa, *m_context); }
    auto get_registers() const noexcept { return make_view(get_data().registers, *m_context); }
    auto get_automata_states() const noexcept { return make_view(get_data().automata_states, *m_context); }
    auto get_rules() const noexcept { return make_view(get_data().rules, *m_context); }
    auto get_load_rules() const noexcept { return make_view(get_data().load_rules, *m_context); }
    auto get_do_rules() const noexcept { return make_view(get_data().do_rules, *m_context); }
    auto get_call_rules() const noexcept { return make_view(get_data().call_rules, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
