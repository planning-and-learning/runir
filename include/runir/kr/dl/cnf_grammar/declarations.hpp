#ifndef RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <concepts>
#include <memory>
#include <type_traits>
#include <variant>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>

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
struct NonTerminal;

template<runir::kr::dl::CategoryTag Category>
struct DerivationRule;

template<runir::kr::dl::CategoryTag Category>
struct SubstitutionRule;

using ConceptTypes = boost::mp11::mp_transform<Concept, runir::kr::dl::GenerationConceptConstructorTags>;

using RoleTypes = boost::mp11::mp_transform<Role, runir::kr::dl::GenerationRoleConstructorTags>;

using BooleanTypes = boost::mp11::mp_transform<Boolean, runir::kr::dl::GenerationBooleanConstructorTags>;

using NumericalTypes = boost::mp11::mp_transform<Numerical, runir::kr::dl::GenerationNumericalConstructorTags>;

/// The records a constructor of Category can be.
template<runir::kr::dl::CategoryTag Category>
using CategoryTypes = std::conditional_t<
    std::same_as<Category, runir::kr::dl::ConceptTag>,
    ConceptTypes,
    std::conditional_t<std::same_as<Category, runir::kr::dl::RoleTag>, RoleTypes, std::conditional_t<std::same_as<Category, runir::kr::dl::BooleanTag>, BooleanTypes, NumericalTypes>>>;

/// A constructor: one of Category's records. Stored as it is, not interned.
template<runir::kr::dl::CategoryTag Category>
using Constructor = urd::index_variant<CategoryTypes<Category>>;

using NonTerminalTypes = boost::mp11::mp_transform<NonTerminal, runir::kr::dl::CategoryTags>;

using DerivationRuleTypes = boost::mp11::mp_transform<DerivationRule, runir::kr::dl::CategoryTags>;

using SubstitutionRuleTypes = boost::mp11::mp_transform<SubstitutionRule, runir::kr::dl::CategoryTags>;

using GrammarTypes = boost::mp11::mp_list<Grammar>;

using ConstructorRepositoryTypes = boost::mp11::mp_append<ConceptTypes,
                                                         RoleTypes,
                                                         BooleanTypes,
                                                         NumericalTypes,
                                                         NonTerminalTypes,
                                                         DerivationRuleTypes,
                                                         SubstitutionRuleTypes,
                                                         GrammarTypes>;

using ConstructorRepository = runir::kr::dl::detail::BasicConstructorRepository<ConstructorRepositoryTypes>;

using ConstructorRepositoryPtr = std::shared_ptr<ConstructorRepository>;

using ConstructorRepositoryFactory = runir::kr::dl::detail::BasicConstructorRepositoryFactory<ConstructorRepositoryTypes>;

template<typename Tag>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
using ConceptView = urd::view<Concept<Tag>, ConstructorRepository>;

template<typename Tag>
    requires runir::kr::dl::RoleConstructorTag<Tag>
using RoleView = urd::view<Role<Tag>, ConstructorRepository>;

template<typename Tag>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
using BooleanView = urd::view<Boolean<Tag>, ConstructorRepository>;

template<typename Tag>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
using NumericalView = urd::view<Numerical<Tag>, ConstructorRepository>;

template<typename T>
using EntityView = urd::view<T, ConstructorRepository>;

/// A constructor read through the repository: the view of the record it holds.
template<runir::kr::dl::CategoryTag Category>
using ConstructorView = boost::mp11::mp_apply<std::variant, boost::mp11::mp_transform<EntityView, CategoryTypes<Category>>>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalView = urd::view<NonTerminal<Category>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleView = urd::view<DerivationRule<Category>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleView = urd::view<SubstitutionRule<Category>, ConstructorRepository>;

using GrammarView = urd::view<Grammar, ConstructorRepository>;
}

#endif
