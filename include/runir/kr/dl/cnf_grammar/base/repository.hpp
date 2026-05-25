#ifndef RUNIR_KR_DL_CNF_GRAMMAR_BASE_REPOSITORY_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/base/declarations.hpp"
#include "runir/kr/dl/cnf_grammar/base/datas.hpp"
#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"

namespace runir::kr::dl::cnf_grammar::base
{

using ConstructorRepository = runir::kr::dl::cnf_grammar::ConstructorRepositoryFor<runir::kr::BaseFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::cnf_grammar::ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>;

template<runir::kr::dl::BaseConceptConstructorTag Tag>
using ConceptView = runir::kr::dl::cnf_grammar::FamilyConceptView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::BaseRoleConstructorTag Tag>
using RoleView = runir::kr::dl::cnf_grammar::FamilyRoleView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::BaseBooleanConstructorTag Tag>
using BooleanView = runir::kr::dl::cnf_grammar::FamilyBooleanView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::BaseNumericalConstructorTag Tag>
using NumericalView = runir::kr::dl::cnf_grammar::FamilyNumericalView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorView = runir::kr::dl::cnf_grammar::FamilyConstructorView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalView = runir::kr::dl::cnf_grammar::FamilyNonTerminalView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleView = runir::kr::dl::cnf_grammar::FamilyDerivationRuleView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleView = runir::kr::dl::cnf_grammar::FamilySubstitutionRuleView<runir::kr::BaseFamilyTag, Category>;

using GrammarView = runir::kr::dl::cnf_grammar::FamilyGrammarView<runir::kr::BaseFamilyTag>;

}  // namespace runir::kr::dl::cnf_grammar::base

#endif
