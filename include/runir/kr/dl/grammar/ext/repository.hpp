#ifndef RUNIR_KR_DL_GRAMMAR_EXT_REPOSITORY_HPP_
#define RUNIR_KR_DL_GRAMMAR_EXT_REPOSITORY_HPP_

#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/ext/datas.hpp"

namespace runir::kr::dl::grammar::ext
{

using ConstructorRepository = runir::kr::dl::grammar::ConstructorRepositoryFor<runir::kr::ExtFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::grammar::ConstructorRepositoryFactoryFor<runir::kr::ExtFamilyTag>;
using GrammarView = runir::kr::dl::grammar::FamilyGrammarView<runir::kr::ExtFamilyTag>;

}  // namespace runir::kr::dl::grammar::ext

#endif
