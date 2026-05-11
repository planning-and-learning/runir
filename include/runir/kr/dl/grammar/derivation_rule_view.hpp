#ifndef RUNIR_GRAMMAR_DERIVATION_RULE_VIEW_HPP_
#define RUNIR_GRAMMAR_DERIVATION_RULE_VIEW_HPP_

#include "runir/kr/dl/grammar/constructor_or_non_terminal_view.hpp"
#include "runir/kr/dl/grammar/derivation_rule_data.hpp"
#include "runir/kr/dl/grammar/non_terminal_view.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/vector.hpp>

namespace tyr
{

template<runir::kr::dl::CategoryTag Category, typename C>
class View<Index<runir::kr::dl::grammar::DerivationRule<Category>>, C>
{
private:
    const C* m_context;
    Index<runir::kr::dl::grammar::DerivationRule<Category>> m_handle;

public:
    View(Index<runir::kr::dl::grammar::DerivationRule<Category>> handle, const C& context) noexcept : m_context(&context), m_handle(handle) {}

    const auto& get_data() const noexcept { return get_repository(*m_context)[m_handle]; }
    const auto& get_context() const noexcept { return *m_context; }
    const auto& get_handle() const noexcept { return m_handle; }

    auto get_index() const noexcept { return m_handle; }
    auto get_lhs() const noexcept { return make_view(get_data().lhs, *m_context); }
    auto get_rhs() const noexcept { return make_view(get_data().rhs, *m_context); }

    auto identifying_members() const noexcept { return std::tie(m_handle, m_context->get_index()); }
};

}  // namespace tyr

#endif
