#ifndef RUNIR_GRAMMAR_CANONICALIZATION_HPP_
#define RUNIR_GRAMMAR_CANONICALIZATION_HPP_

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/numerical_data.hpp"
#include "runir/kr/dl/grammar/derivation_rule_data.hpp"
#include "runir/kr/dl/grammar/grammar_data.hpp"
#include "runir/kr/dl/grammar/non_terminal_data.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

#include <algorithm>
#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::dl::grammar
{

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const urd::data<NonTerminal<Category>>&) noexcept
{ return true; }

template<runir::kr::dl::CategoryTag Category>
void canonicalize(urd::data<NonTerminal<Category>>&) noexcept
{
}

template<runir::kr::dl::CategoryTag Category>
inline bool is_canonical(const urd::data<DerivationRule<Category>>& data)
{ return ygg::is_canonical(data.rhs); }

template<runir::kr::dl::CategoryTag Category>
inline void canonicalize(urd::data<DerivationRule<Category>>& data)
{ ygg::canonicalize(data.rhs); }

inline bool is_canonical(const urd::data<GrammarTag>& data)
{
    return ygg::is_canonical(data.concept_derivation_rules) && ygg::is_canonical(data.role_derivation_rules) && ygg::is_canonical(data.boolean_derivation_rules)
           && ygg::is_canonical(data.numerical_derivation_rules);
}

inline void canonicalize(urd::data<GrammarTag>& data)
{
    ygg::canonicalize(data.concept_derivation_rules);
    ygg::canonicalize(data.role_derivation_rules);
    ygg::canonicalize(data.boolean_derivation_rules);
    ygg::canonicalize(data.numerical_derivation_rules);
}

}  // namespace runir::kr::dl::grammar

#endif
