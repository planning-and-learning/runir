#ifndef RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_VIEW_HPP_
#define RUNIR_GRAMMAR_CONSTRUCTOR_OR_NON_TERMINAL_VIEW_HPP_

#include "runir/grammar/constructor_or_non_terminal_data.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>

namespace tyr
{

template<runir::CategoryTag Category, typename C>
class View<Index<runir::grammar::ConstructorOrNonTerminal<Category>>, C>
{
private:
    const C* m_context;
    Index<runir::grammar::ConstructorOrNonTerminal<Category>> m_handle;

public:
    View(Index<runir::grammar::ConstructorOrNonTerminal<Category>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_variant() const noexcept { return make_view(get_data().value, *m_context); }

    auto identifying_members() const noexcept { return std::tie(get_data()); }
};

}  // namespace tyr

#endif
