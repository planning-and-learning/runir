#ifndef RUNIR_KR_DL_CNF_GRAMMAR_TRANSLATE_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_TRANSLATE_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

namespace runir::kr::dl::cnf_grammar
{

template<runir::kr::dl::FamilyTag Family>
FamilyGrammarView<Family> translate(const runir::kr::dl::grammar::FamilyGrammarView<runir::kr::BaseFamilyTag>& grammar,
                                    ConstructorRepositoryFor<Family>& repository);

FamilyGrammarView<runir::kr::BaseFamilyTag> translate(const runir::kr::dl::grammar::FamilyGrammarView<runir::kr::BaseFamilyTag>& grammar,
                                                      ConstructorRepositoryFor<runir::kr::BaseFamilyTag>& repository);

}  // namespace runir::kr::dl::cnf_grammar

#endif
