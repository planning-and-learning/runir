#ifndef RUNIR_CNF_GRAMMAR_CANONICALIZATION_HPP_
#define RUNIR_CNF_GRAMMAR_CANONICALIZATION_HPP_

#include "runir/kr/dl/canonicalization.hpp"
#include "runir/kr/dl/numerical_data.hpp"
#include "runir/kr/dl/cnf_grammar/derivation_rule_data.hpp"
#include "runir/kr/dl/cnf_grammar/grammar_data.hpp"
#include "runir/kr/dl/cnf_grammar/substitution_rule_data.hpp"

#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::dl::cnf_grammar
{

template<runir::kr::dl::CategoryTag Category>
bool is_canonical(const urd::data<NonTerminal<Category>>&) noexcept
{ return true; }

template<runir::kr::dl::CategoryTag Category>
void canonicalize(urd::data<NonTerminal<Category>>&) noexcept
{
    // Trivially canonical
}

template<runir::kr::dl::CategoryTag Category>
inline bool is_canonical(const urd::data<DerivationRule<Category>>&) noexcept
{ return true; }

template<runir::kr::dl::CategoryTag Category>
inline void canonicalize(urd::data<DerivationRule<Category>>&) noexcept
{
}

template<runir::kr::dl::CategoryTag Category>
inline bool is_canonical(const urd::data<SubstitutionRule<Category>>&) noexcept
{ return true; }

template<runir::kr::dl::CategoryTag Category>
inline void canonicalize(urd::data<SubstitutionRule<Category>>&) noexcept
{
}

inline bool is_canonical(const urd::data<Grammar>& data)
{
    return ygg::is_canonical(data.concept_derivation_rules) && ygg::is_canonical(data.role_derivation_rules) && ygg::is_canonical(data.boolean_derivation_rules)
           && ygg::is_canonical(data.numerical_derivation_rules) && ygg::is_canonical(data.concept_substitution_rules)
           && ygg::is_canonical(data.role_substitution_rules) && ygg::is_canonical(data.boolean_substitution_rules)
           && ygg::is_canonical(data.numerical_substitution_rules);
}

inline void canonicalize(urd::data<Grammar>& data)
{
    ygg::canonicalize(data.concept_derivation_rules);
    ygg::canonicalize(data.role_derivation_rules);
    ygg::canonicalize(data.boolean_derivation_rules);
    ygg::canonicalize(data.numerical_derivation_rules);
    ygg::canonicalize(data.concept_substitution_rules);
    ygg::canonicalize(data.role_substitution_rules);
    ygg::canonicalize(data.boolean_substitution_rules);
    ygg::canonicalize(data.numerical_substitution_rules);
}

}  // namespace runir::kr::dl::cnf_grammar

#endif
