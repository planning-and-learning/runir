#ifndef RUNIR_CNF_GRAMMAR_TRANSLATE_HPP_
#define RUNIR_CNF_GRAMMAR_TRANSLATE_HPP_

#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"

namespace runir::kr::dl::cnf_grammar
{

GrammarView translate(const runir::kr::dl::grammar::GrammarView& grammar, ConstructorRepository& repository);

}

#endif
