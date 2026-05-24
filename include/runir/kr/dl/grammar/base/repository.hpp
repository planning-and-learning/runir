#ifndef RUNIR_KR_DL_GRAMMAR_BASE_REPOSITORY_HPP_
#define RUNIR_KR_DL_GRAMMAR_BASE_REPOSITORY_HPP_

#include "runir/kr/dl/grammar/base/datas.hpp"
#include "runir/kr/dl/grammar/constructor_repository.hpp"

#include <memory>
#include <vector>

namespace runir::kr::dl::grammar::base
{

using ConstructorRepositoryTypes = runir::kr::dl::grammar::FamilyConstructorRepositoryTypes<runir::kr::dl::BaseFamilyTag>;
using ConstructorSymbolRepository = runir::kr::dl::grammar::FamilyConstructorSymbolRepository<runir::kr::dl::BaseFamilyTag>;
using ConstructorRepository = runir::kr::dl::grammar::ConstructorRepositoryFor<runir::kr::dl::BaseFamilyTag>;
using ConstructorRepositoryPtr = runir::kr::dl::grammar::ConstructorRepositoryPtrFor<runir::kr::dl::BaseFamilyTag>;
using ConstructorRepositoryFactory = runir::kr::dl::grammar::ConstructorRepositoryFactoryFor<runir::kr::dl::BaseFamilyTag>;
using ConstructorRepositoryFactoryPtr = std::shared_ptr<ConstructorRepositoryFactory>;

template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptView = runir::kr::dl::grammar::FamilyConceptView<runir::kr::dl::BaseFamilyTag, Tag>;

template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptListView = tyr::View<tyr::IndexList<Concept<runir::kr::dl::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::ConceptConstructorTag Tag>
using ConceptViewList = std::vector<ConceptView<Tag>>;

template<runir::kr::dl::RoleConstructorTag Tag>
using RoleView = runir::kr::dl::grammar::FamilyRoleView<runir::kr::dl::BaseFamilyTag, Tag>;

template<runir::kr::dl::RoleConstructorTag Tag>
using RoleListView = tyr::View<tyr::IndexList<Role<runir::kr::dl::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::RoleConstructorTag Tag>
using RoleViewList = std::vector<RoleView<Tag>>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanView = runir::kr::dl::grammar::FamilyBooleanView<runir::kr::dl::BaseFamilyTag, Tag>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanListView = tyr::View<tyr::IndexList<Boolean<runir::kr::dl::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::BooleanConstructorTag Tag>
using BooleanViewList = std::vector<BooleanView<Tag>>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalView = runir::kr::dl::grammar::FamilyNumericalView<runir::kr::dl::BaseFamilyTag, Tag>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalListView = tyr::View<tyr::IndexList<Numerical<runir::kr::dl::BaseFamilyTag, Tag>>, ConstructorRepository>;

template<runir::kr::dl::NumericalConstructorTag Tag>
using NumericalViewList = std::vector<NumericalView<Tag>>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorView = runir::kr::dl::grammar::FamilyConstructorView<runir::kr::dl::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorListView = tyr::View<tyr::IndexList<Constructor<runir::kr::dl::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorViewList = std::vector<ConstructorView<Category>>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorOrNonTerminalView = runir::kr::dl::grammar::FamilyConstructorOrNonTerminalView<runir::kr::dl::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorOrNonTerminalListView = tyr::View<tyr::IndexList<ConstructorOrNonTerminal<runir::kr::dl::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorOrNonTerminalViewList = std::vector<ConstructorOrNonTerminalView<Category>>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalView = runir::kr::dl::grammar::FamilyNonTerminalView<runir::kr::dl::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalListView = tyr::View<tyr::IndexList<NonTerminal<runir::kr::dl::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalViewList = std::vector<NonTerminalView<Category>>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleView = runir::kr::dl::grammar::FamilyDerivationRuleView<runir::kr::dl::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleListView = tyr::View<tyr::IndexList<DerivationRule<runir::kr::dl::BaseFamilyTag, Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleViewList = std::vector<DerivationRuleView<Category>>;

using GrammarView = runir::kr::dl::grammar::FamilyGrammarView<runir::kr::dl::BaseFamilyTag>;
using GrammarListView = tyr::View<tyr::IndexList<GrammarTag<runir::kr::dl::BaseFamilyTag>>, ConstructorRepository>;
using GrammarViewList = std::vector<GrammarView>;

}  // namespace runir::kr::dl::grammar::base

#endif
