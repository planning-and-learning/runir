#ifndef RUNIR_CNF_GRAMMAR_NON_TERMINAL_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_NON_TERMINAL_VIEW_HPP_

#include "runir/kr/dl/cnf_grammar/non_terminal_data.hpp"
#include "runir/kr/dl/detail/grammar_views.hpp"

namespace runir::kr::dl::detail
{
template<CategoryTag Category>
inline constexpr bool is_grammar_non_terminal<cnf_grammar::NonTerminal<Category>> = true;
}  // namespace runir::kr::dl::detail

#endif
