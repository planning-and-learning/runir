#ifndef RUNIR_KR_DL_CNF_GRAMMAR_TRANSLATE_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_TRANSLATE_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

namespace runir::kr::dl::cnf_grammar
{

GrammarView translate(const runir::kr::dl::grammar::GrammarView& grammar, ConstructorRepository& repository);

}  // namespace runir::kr::dl::cnf_grammar

#endif
