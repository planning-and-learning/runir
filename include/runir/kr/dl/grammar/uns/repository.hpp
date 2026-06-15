#ifndef RUNIR_KR_DL_GRAMMAR_UNS_REPOSITORY_HPP_
#define RUNIR_KR_DL_GRAMMAR_UNS_REPOSITORY_HPP_

#include "runir/kr/dl/grammar/constructor_repository.hpp"
#include "runir/kr/dl/grammar/uns/datas.hpp"

namespace runir::kr::dl::grammar::uns
{

using ConstructorRepository = runir::kr::dl::grammar::ConstructorRepositoryFor<runir::kr::UnsFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::grammar::ConstructorRepositoryFactoryFor<runir::kr::UnsFamilyTag>;
using GrammarView = runir::kr::dl::grammar::FamilyGrammarView<runir::kr::UnsFamilyTag>;

}  // namespace runir::kr::dl::grammar::uns

#endif
