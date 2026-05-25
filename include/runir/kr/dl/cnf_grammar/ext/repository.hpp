#ifndef RUNIR_KR_DL_CNF_GRAMMAR_EXT_REPOSITORY_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_EXT_REPOSITORY_HPP_

#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/cnf_grammar/ext/datas.hpp"

namespace runir::kr::dl::cnf_grammar::ext
{

using ConstructorRepository = runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<runir::kr::ExtFamilyTag>;
using GrammarView = runir::kr::dl::cnf_grammar::FamilyGrammarView<runir::kr::ExtFamilyTag>;

}  // namespace runir::kr::dl::cnf_grammar::ext

#endif
