#ifndef RUNIR_CNF_GRAMMAR_TRANSLATE_HPP_
#define RUNIR_CNF_GRAMMAR_TRANSLATE_HPP_

#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/base/repository.hpp"

namespace runir::kr::dl::cnf_grammar
{

template<runir::kr::dl::FamilyTag Family>
FamilyGrammarView<Family> translate(const runir::kr::dl::grammar::base::GrammarView& grammar, ConstructorRepositoryFor<Family>& repository);

}

#endif
