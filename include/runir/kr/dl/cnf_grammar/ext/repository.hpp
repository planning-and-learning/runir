#ifndef RUNIR_KR_DL_CNF_GRAMMAR_EXT_REPOSITORY_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_EXT_REPOSITORY_HPP_

#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"
#include "runir/kr/dl/cnf_grammar/ext/datas.hpp"

#include <memory>

namespace runir::kr::dl::cnf_grammar::ext
{

using ConstructorRepositoryTypes = runir::kr::dl::cnf_grammar::FamilyConstructorRepositoryTypes<runir::kr::ExtFamilyTag>;
using ConstructorSymbolRepository = runir::kr::dl::cnf_grammar::FamilyConstructorSymbolRepository<runir::kr::ExtFamilyTag>;
using ConstructorRepository = runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>;
using ConstructorRepositoryPtr = runir::kr::dl::cnf_grammar::ConstructorRepositoryPtrFor<runir::kr::ExtFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<runir::kr::ExtFamilyTag>;
using ConstructorRepositoryFactoryPtr = std::shared_ptr<ConstructorRepositoryFactory>;
using GrammarView = runir::kr::dl::cnf_grammar::FamilyGrammarView<runir::kr::ExtFamilyTag>;

}  // namespace runir::kr::dl::cnf_grammar::ext

#endif
