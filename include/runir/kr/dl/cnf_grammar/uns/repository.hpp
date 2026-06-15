#ifndef RUNIR_KR_DL_CNF_GRAMMAR_UNS_REPOSITORY_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_UNS_REPOSITORY_HPP_

#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/cnf_grammar/uns/datas.hpp"

namespace runir::kr::dl::cnf_grammar::uns
{

using ConstructorRepository = runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<runir::kr::UnsFamilyTag>;
using GrammarView = runir::kr::dl::cnf_grammar::FamilyGrammarView<runir::kr::UnsFamilyTag>;

}  // namespace runir::kr::dl::cnf_grammar::uns

#endif
