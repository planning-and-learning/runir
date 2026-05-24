#ifndef RUNIR_KR_DL_CNF_GRAMMAR_BASE_REPOSITORY_HPP_
#define RUNIR_KR_DL_CNF_GRAMMAR_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/cnf_grammar/base/datas.hpp"
#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"

#include <memory>
#include <vector>

namespace runir::kr::dl::cnf_grammar::base
{

using ConstructorRepository = ConstructorRepositoryFor<runir::kr::BaseFamilyTag>;
using ConstructorRepositoryPtr = ConstructorRepositoryPtrFor<runir::kr::BaseFamilyTag>;
using ConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>;
using ConstructorRepositoryFactoryPtr = std::shared_ptr<ConstructorRepositoryFactory>;

template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptView = FamilyConceptView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptListView = tyr::View<tyr::IndexList<Concept<runir::kr::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptViewList = std::vector<ConceptView<Tag>>;

template<runir::kr::dl::RoleConstructorTag Tag>
using RoleView = FamilyRoleView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::RoleConstructorTag Tag>
using RoleListView = tyr::View<tyr::IndexList<Role<runir::kr::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::RoleConstructorTag Tag>
using RoleViewList = std::vector<RoleView<Tag>>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanView = FamilyBooleanView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanListView = tyr::View<tyr::IndexList<Boolean<runir::kr::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanViewList = std::vector<BooleanView<Tag>>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalView = FamilyNumericalView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalListView = tyr::View<tyr::IndexList<Numerical<runir::kr::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalViewList = std::vector<NumericalView<Tag>>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorView = FamilyConstructorView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorListView = tyr::View<tyr::IndexList<Constructor<runir::kr::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorViewList = std::vector<ConstructorView<Category>>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalView = FamilyNonTerminalView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalListView = tyr::View<tyr::IndexList<NonTerminal<runir::kr::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalViewList = std::vector<NonTerminalView<Category>>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleView = FamilyDerivationRuleView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleListView = tyr::View<tyr::IndexList<DerivationRule<runir::kr::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleViewList = std::vector<DerivationRuleView<Category>>;

template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleView = FamilySubstitutionRuleView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleListView = tyr::View<tyr::IndexList<SubstitutionRule<runir::kr::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleViewList = std::vector<SubstitutionRuleView<Category>>;

using GrammarView = FamilyGrammarView<runir::kr::BaseFamilyTag>;

}  // namespace runir::kr::dl::cnf_grammar::base

#endif
