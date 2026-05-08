#ifndef RUNIR_CNF_GRAMMAR_TRANSLATE_HPP_
#define RUNIR_CNF_GRAMMAR_TRANSLATE_HPP_

#include "runir/cnf_grammar/constructor_repository.hpp"
#include "runir/grammar/constructor_repository.hpp"

namespace runir::cnf_grammar
{

GrammarView translate(const runir::grammar::GrammarView& grammar, ConstructorRepository& repository);

}

#endif
