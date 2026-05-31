#ifndef RUNIR_CNF_GRAMMAR_NON_TERMINAL_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_NON_TERMINAL_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/non_terminal_data.hpp"

#include <tuple>
#include <yggdrasil/core/types.hpp>

namespace ygg
{

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>> m_handle;

public:
    View(Index<runir::kr::dl::cnf_grammar::NonTerminal<Family, Category>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    const auto& get_name() const noexcept { return get_data().name; }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace ygg

#endif
