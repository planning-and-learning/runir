#ifndef RUNIR_KR_DL_CNF_GRAMMAR_BASE_TRANSLATE_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_BASE_TRANSLATE_HPP_

#include "runir/kr/dl/cnf_grammar/base/repository.hpp"
#include "runir/kr/dl/grammar/base/repository.hpp"

namespace runir::kr::dl::cnf_grammar::base
{

template<runir::kr::dl::FamilyTag Family>
FamilyGrammarView<Family> translate(const runir::kr::dl::grammar::base::GrammarView& grammar, ConstructorRepositoryFor<Family>& repository);

GrammarView translate(const runir::kr::dl::grammar::base::GrammarView& grammar, ConstructorRepository& repository);

}  // namespace runir::kr::dl::cnf_grammar::base

#endif
