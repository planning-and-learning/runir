#ifndef RUNIR_KR_DL_CNF_GRAMMAR_EXT_REPOSITORY_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_EXT_REPOSITORY_HPP_

#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"

namespace runir::kr::dl::cnf_grammar::ext
{

using ConstructorRepositoryTypes = runir::kr::dl::cnf_grammar::FamilyConstructorRepositoryTypes<runir::kr::ExtFamilyTag>;
using ConstructorSymbolRepository = runir::kr::dl::cnf_grammar::FamilyConstructorSymbolRepository<runir::kr::ExtFamilyTag>;
using ConstructorRepository = runir::kr::dl::cnf_grammar::ExtConstructorRepository;
using ConstructorRepositoryFactory = runir::kr::dl::cnf_grammar::ExtConstructorRepositoryFactory;
using ConstructorRepositoryFactoryPtr = runir::kr::dl::cnf_grammar::ExtConstructorRepositoryFactoryPtr;
using GrammarView = runir::kr::dl::cnf_grammar::FamilyGrammarView<runir::kr::ExtFamilyTag>;

}  // namespace runir::kr::dl::cnf_grammar::ext

#endif
