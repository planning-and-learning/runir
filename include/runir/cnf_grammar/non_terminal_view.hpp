#ifndef RUNIR_CNF_GRAMMAR_NON_TERMINAL_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_NON_TERMINAL_VIEW_HPP_

#include "runir/cnf_grammar/non_terminal_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>

namespace tyr
{

template<runir::CategoryTag Category, typename C>
class View<Index<runir::cnf_grammar::NonTerminal<Category>>, C>
{
private:
    const C* m_context;
    Index<runir::cnf_grammar::NonTerminal<Category>> m_handle;

public:
    View(Index<runir::cnf_grammar::NonTerminal<Category>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    const auto& get_name() const noexcept { return get_data().name; }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
