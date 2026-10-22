#ifndef RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <memory>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::cnf_grammar
{

struct Grammar;

template<typename Tag>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
struct Concept;

template<typename Tag>
    requires runir::kr::dl::RoleConstructorTag<Tag>
struct Role;

template<typename Tag>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
struct Boolean;

template<typename Tag>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
struct Numerical;

template<runir::kr::dl::CategoryTag Category>
struct Constructor;

template<runir::kr::dl::CategoryTag Category>
struct NonTerminal;

template<runir::kr::dl::CategoryTag Category>
struct DerivationRule;

template<runir::kr::dl::CategoryTag Category>
struct SubstitutionRule;

using ConceptTypes = ygg::MapTypeListT<Concept, runir::kr::dl::GenerationConceptConstructorTags>;

using RoleTypes = ygg::MapTypeListT<Role, runir::kr::dl::GenerationRoleConstructorTags>;

using BooleanTypes = ygg::MapTypeListT<Boolean, runir::kr::dl::GenerationBooleanConstructorTags>;

using NumericalTypes = ygg::MapTypeListT<Numerical, runir::kr::dl::GenerationNumericalConstructorTags>;

using ConstructorTypes = ygg::MapTypeListT<Constructor, runir::kr::dl::CategoryTags>;

using NonTerminalTypes = ygg::MapTypeListT<NonTerminal, runir::kr::dl::CategoryTags>;

using DerivationRuleTypes = ygg::MapTypeListT<DerivationRule, runir::kr::dl::CategoryTags>;

using SubstitutionRuleTypes = ygg::MapTypeListT<SubstitutionRule, runir::kr::dl::CategoryTags>;

using GrammarTypes = ygg::TypeList<Grammar>;

using ConstructorRepositoryTypes = ygg::ConcatTypeListsT<ConceptTypes,
                                                         RoleTypes,
                                                         BooleanTypes,
                                                         NumericalTypes,
                                                         ConstructorTypes,
                                                         NonTerminalTypes,
                                                         DerivationRuleTypes,
                                                         SubstitutionRuleTypes,
                                                         GrammarTypes>;

using ConstructorRepository = runir::kr::dl::detail::BasicConstructorRepository<ConstructorRepositoryTypes>;

using ConstructorRepositoryPtr = std::shared_ptr<ConstructorRepository>;

using ConstructorRepositoryFactory = runir::kr::dl::detail::BasicConstructorRepositoryFactory<ConstructorRepositoryTypes>;

template<typename Tag>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
using ConceptView = ygg::View<ygg::Index<Concept<Tag>>, ConstructorRepository>;

template<typename Tag>
    requires runir::kr::dl::RoleConstructorTag<Tag>
using RoleView = ygg::View<ygg::Index<Role<Tag>>, ConstructorRepository>;

template<typename Tag>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
using BooleanView = ygg::View<ygg::Index<Boolean<Tag>>, ConstructorRepository>;

template<typename Tag>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
using NumericalView = ygg::View<ygg::Index<Numerical<Tag>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using ConstructorView = ygg::View<ygg::Index<Constructor<Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalView = ygg::View<ygg::Index<NonTerminal<Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleView = ygg::View<ygg::Index<DerivationRule<Category>>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleView = ygg::View<ygg::Index<SubstitutionRule<Category>>, ConstructorRepository>;

using GrammarView = ygg::View<ygg::Index<Grammar>, ConstructorRepository>;
}

#endif
