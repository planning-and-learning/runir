#ifndef RUNIR_CNF_GRAMMAR_DERIVATION_RULE_VIEW_HPP_
#define RUNIR_CNF_GRAMMAR_DERIVATION_RULE_VIEW_HPP_

#include "runir/kr/dl/detail/grammar_views.hpp"
#include "runir/kr/dl/cnf_grammar/derivation_rule_data.hpp"
#include "runir/kr/dl/cnf_grammar/non_terminal_view.hpp"

namespace runir::kr::dl::detail
{
template<CategoryTag Category>
inline constexpr bool is_grammar_derivation_rule<cnf_grammar::DerivationRule<Category>> = true;
}  // namespace runir::kr::dl::detail

#endif
