#ifndef RUNIR_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_GRAMMAR_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <memory>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::grammar
{

template<typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Tag>
struct Concept;

template<typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Tag>
struct Role;

template<typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Tag>
struct Boolean;

template<typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Tag>
struct Numerical;

template<runir::kr::dl::CategoryTag Category>
struct Constructor;

template<runir::kr::dl::CategoryTag Category>
struct NonTerminal;

template<runir::kr::dl::CategoryTag Category>
struct ConstructorOrNonTerminal;

template<runir::kr::dl::CategoryTag Category>
struct DerivationRule;

struct GrammarTag;

using FamilyConceptTypes = ygg::MapTypeListT<Concept, runir::kr::dl::GenerationConceptConstructorTags>;

using FamilyRoleTypes = ygg::MapTypeListT<Role, runir::kr::dl::GenerationRoleConstructorTags>;

using FamilyBooleanTypes = ygg::MapTypeListT<Boolean, runir::kr::dl::GenerationBooleanConstructorTags>;

using FamilyNumericalTypes = ygg::MapTypeListT<Numerical, runir::kr::dl::GenerationNumericalConstructorTags>;

using FamilyConstructorTypes = ygg::MapTypeListT<Constructor, runir::kr::dl::CategoryTags>;

using FamilyConstructorOrNonTerminalTypes = ygg::MapTypeListT<ConstructorOrNonTerminal, runir::kr::dl::CategoryTags>;

using FamilyNonTerminalTypes = ygg::MapTypeListT<NonTerminal, runir::kr::dl::CategoryTags>;

using FamilyDerivationRuleTypes = ygg::MapTypeListT<DerivationRule, runir::kr::dl::CategoryTags>;

using FamilyGrammarTypes = ygg::TypeList<GrammarTag>;

using FamilyConstructorRepositoryTypes = ygg::ConcatTypeListsT<FamilyConceptTypes,
                                                               FamilyRoleTypes,
                                                               FamilyBooleanTypes,
                                                               FamilyNumericalTypes,
                                                               FamilyConstructorTypes,
                                                               FamilyConstructorOrNonTerminalTypes,
                                                               FamilyNonTerminalTypes,
                                                               FamilyDerivationRuleTypes,
                                                               FamilyGrammarTypes>;

using BasicConstructorRepository = runir::kr::dl::detail::ConstructorRepository<FamilyConstructorRepositoryTypes>;

using BasicConstructorRepositoryFactory = runir::kr::dl::detail::ConstructorRepositoryFactory<FamilyConstructorRepositoryTypes>;

using ConstructorRepositoryFor = BasicConstructorRepository;

using ConstructorRepositoryPtrFor = std::shared_ptr<ConstructorRepositoryFor>;

using ConstructorRepositoryFactoryFor = BasicConstructorRepositoryFactory;

template<typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Tag>
using FamilyConceptView = ygg::View<ygg::Index<Concept<Tag>>, ConstructorRepositoryFor>;

template<typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Tag>
using FamilyRoleView = ygg::View<ygg::Index<Role<Tag>>, ConstructorRepositoryFor>;

template<typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Tag>
using FamilyBooleanView = ygg::View<ygg::Index<Boolean<Tag>>, ConstructorRepositoryFor>;

template<typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Tag>
using FamilyNumericalView = ygg::View<ygg::Index<Numerical<Tag>>, ConstructorRepositoryFor>;

template<runir::kr::dl::CategoryTag Category>
using FamilyConstructorView = ygg::View<ygg::Index<Constructor<Category>>, ConstructorRepositoryFor>;

template<runir::kr::dl::CategoryTag Category>
using FamilyConstructorOrNonTerminalView = ygg::View<ygg::Index<ConstructorOrNonTerminal<Category>>, ConstructorRepositoryFor>;

template<runir::kr::dl::CategoryTag Category>
using FamilyNonTerminalView = ygg::View<ygg::Index<NonTerminal<Category>>, ConstructorRepositoryFor>;

template<runir::kr::dl::CategoryTag Category>
using FamilyDerivationRuleView = ygg::View<ygg::Index<DerivationRule<Category>>, ConstructorRepositoryFor>;

using FamilyGrammarView = ygg::View<ygg::Index<GrammarTag>, ConstructorRepositoryFor>;

using BaseConstructorRepository = ConstructorRepositoryFor;
using BaseConstructorRepositoryFactory = ConstructorRepositoryFactoryFor;
using BaseGrammarView = FamilyGrammarView;

using ExtConstructorRepository = ConstructorRepositoryFor;
using ExtConstructorRepositoryFactory = ConstructorRepositoryFactoryFor;
using ExtGrammarView = FamilyGrammarView;

using UnsConstructorRepository = ConstructorRepositoryFor;
using UnsConstructorRepositoryFactory = ConstructorRepositoryFactoryFor;
using UnsGrammarView = FamilyGrammarView;

template<typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Tag>
using BaseConceptView = FamilyConceptView<Tag>;

template<typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Tag>
using BaseRoleView = FamilyRoleView<Tag>;

template<typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Tag>
using BaseBooleanView = FamilyBooleanView<Tag>;

template<typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Tag>
using BaseNumericalView = FamilyNumericalView<Tag>;

template<runir::kr::dl::CategoryTag Category>
using BaseConstructorView = FamilyConstructorView<Category>;

template<runir::kr::dl::CategoryTag Category>
using BaseConstructorOrNonTerminalView = FamilyConstructorOrNonTerminalView<Category>;

template<runir::kr::dl::CategoryTag Category>
using BaseNonTerminalView = FamilyNonTerminalView<Category>;

template<runir::kr::dl::CategoryTag Category>
using BaseDerivationRuleView = FamilyDerivationRuleView<Category>;

}  // namespace runir::kr::dl::grammar

#endif
