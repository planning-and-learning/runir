#ifndef RUNIR_KR_DL_CNF_GRAMMAR_TRANSLATE_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_TRANSLATE_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

namespace runir::kr::dl::cnf_grammar
{

FamilyGrammarView translate(const runir::kr::dl::grammar::FamilyGrammarView& grammar, ConstructorRepositoryFor& repository);

}  // namespace runir::kr::dl::cnf_grammar

#endif
