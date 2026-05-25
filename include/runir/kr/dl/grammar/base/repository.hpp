#ifndef RUNIR_KR_DL_GRAMMAR_BASE_REPOSITORY_HPP_
#define RUNIR_KR_DL_GRAMMAR_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/base/declarations.hpp"
#include "runir/kr/dl/grammar/base/datas.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"

namespace runir::kr::dl::grammar::base
{

using ConstructorRepository = runir::kr::dl::grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::grammar::ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>;

template<runir::kr::dl::BaseConceptConstructorTag Tag>
using ConceptView = runir::kr::dl::grammar::FamilyConceptView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::BaseRoleConstructorTag Tag>
using RoleView = runir::kr::dl::grammar::FamilyRoleView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::BaseBooleanConstructorTag Tag>
using BooleanView = runir::kr::dl::grammar::FamilyBooleanView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::BaseNumericalConstructorTag Tag>
using NumericalView = runir::kr::dl::grammar::FamilyNumericalView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorView = runir::kr::dl::grammar::FamilyConstructorView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorOrNonTerminalView = runir::kr::dl::grammar::FamilyConstructorOrNonTerminalView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalView = runir::kr::dl::grammar::FamilyNonTerminalView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleView = runir::kr::dl::grammar::FamilyDerivationRuleView<runir::kr::BaseFamilyTag, Category>;

using GrammarView = runir::kr::dl::grammar::FamilyGrammarView<runir::kr::BaseFamilyTag>;

}  // namespace runir::kr::dl::grammar::base

#endif
