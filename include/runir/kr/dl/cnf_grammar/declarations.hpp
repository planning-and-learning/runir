#ifndef RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <memory>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::cnf_grammar
{

template<runir::kr::dl::FamilyTag Family>
struct Grammar;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
struct Concept;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
struct Role;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
struct Boolean;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
struct Numerical;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Constructor;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct NonTerminal;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct DerivationRule;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct SubstitutionRule;

template<runir::kr::dl::FamilyTag Family>
using FamilyConceptTypes = ygg::MapTypeListSecondT<Concept, Family, runir::kr::dl::FamilyConceptConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyRoleTypes = ygg::MapTypeListSecondT<Role, Family, runir::kr::dl::FamilyRoleConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyBooleanTypes = ygg::MapTypeListSecondT<Boolean, Family, runir::kr::dl::FamilyBooleanConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyNumericalTypes = ygg::MapTypeListSecondT<Numerical, Family, runir::kr::dl::FamilyNumericalConstructorTags<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorTypes = ygg::MapTypeListSecondT<Constructor, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyNonTerminalTypes = ygg::MapTypeListSecondT<NonTerminal, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyDerivationRuleTypes = ygg::MapTypeListSecondT<DerivationRule, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilySubstitutionRuleTypes = ygg::MapTypeListSecondT<SubstitutionRule, Family, runir::kr::dl::CategoryTags>;

template<runir::kr::dl::FamilyTag Family>
using FamilyGrammarTypes = ygg::TypeList<Grammar<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyConstructorRepositoryTypes = ygg::ConcatTypeListsT<FamilyConceptTypes<Family>,
                                                               FamilyRoleTypes<Family>,
                                                               FamilyBooleanTypes<Family>,
                                                               FamilyNumericalTypes<Family>,
                                                               FamilyConstructorTypes<Family>,
                                                               FamilyNonTerminalTypes<Family>,
                                                               FamilyDerivationRuleTypes<Family>,
                                                               FamilySubstitutionRuleTypes<Family>,
                                                               FamilyGrammarTypes<Family>>;

template<runir::kr::dl::FamilyTag Family>
using BasicConstructorRepository = runir::kr::dl::detail::ConstructorRepository<Family, FamilyConstructorRepositoryTypes<Family>>;

template<runir::kr::dl::FamilyTag Family>
using BasicConstructorRepositoryFactory = runir::kr::dl::detail::ConstructorRepositoryFactory<Family, FamilyConstructorRepositoryTypes<Family>>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryFor = BasicConstructorRepository<Family>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryPtrFor = std::shared_ptr<ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family>
using ConstructorRepositoryFactoryFor = BasicConstructorRepositoryFactory<Family>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
using FamilyConceptView = ygg::View<ygg::Index<Concept<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
using FamilyRoleView = ygg::View<ygg::Index<Role<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
using FamilyBooleanView = ygg::View<ygg::Index<Boolean<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
using FamilyNumericalView = ygg::View<ygg::Index<Numerical<Family, Tag>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyConstructorView = ygg::View<ygg::Index<Constructor<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyNonTerminalView = ygg::View<ygg::Index<NonTerminal<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilyDerivationRuleView = ygg::View<ygg::Index<DerivationRule<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
using FamilySubstitutionRuleView = ygg::View<ygg::Index<SubstitutionRule<Family, Category>>, ConstructorRepositoryFor<Family>>;

template<runir::kr::dl::FamilyTag Family>
using FamilyGrammarView = ygg::View<ygg::Index<Grammar<Family>>, ConstructorRepositoryFor<Family>>;

using BaseConstructorRepository = ConstructorRepositoryFor<runir::kr::BaseFamilyTag>;
using BaseConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::BaseFamilyTag>;
using BaseGrammarView = FamilyGrammarView<runir::kr::BaseFamilyTag>;

using ExtConstructorRepository = ConstructorRepositoryFor<runir::kr::ExtFamilyTag>;
using ExtConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::ExtFamilyTag>;
using ExtGrammarView = FamilyGrammarView<runir::kr::ExtFamilyTag>;

using UnsConstructorRepository = ConstructorRepositoryFor<runir::kr::UnsFamilyTag>;
using UnsConstructorRepositoryFactory = ConstructorRepositoryFactoryFor<runir::kr::UnsFamilyTag>;
using UnsGrammarView = FamilyGrammarView<runir::kr::UnsFamilyTag>;

template<typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseConceptView = FamilyConceptView<runir::kr::BaseFamilyTag, Tag>;

template<typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseRoleView = FamilyRoleView<runir::kr::BaseFamilyTag, Tag>;

template<typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseBooleanView = FamilyBooleanView<runir::kr::BaseFamilyTag, Tag>;

template<typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<runir::kr::BaseFamilyTag, Tag>
using BaseNumericalView = FamilyNumericalView<runir::kr::BaseFamilyTag, Tag>;

template<runir::kr::dl::CategoryTag Category>
using BaseConstructorView = FamilyConstructorView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using BaseNonTerminalView = FamilyNonTerminalView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using BaseDerivationRuleView = FamilyDerivationRuleView<runir::kr::BaseFamilyTag, Category>;

template<runir::kr::dl::CategoryTag Category>
using BaseSubstitutionRuleView = FamilySubstitutionRuleView<runir::kr::BaseFamilyTag, Category>;

}

#endif
