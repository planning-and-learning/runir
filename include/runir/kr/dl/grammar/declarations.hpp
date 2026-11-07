#ifndef RUNIR_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_GRAMMAR_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <cista/containers/variant.h>
#include <concepts>
#include <memory>
#include <type_traits>
#include <variant>
#include <urd/elements/flat.hpp>
#include <boost/mp11.hpp>

namespace runir::kr::dl::grammar
{

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

struct GrammarTag;

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

/// A grammar symbol of Category: a non-terminal or a constructor. Stored as it is, not interned.
template<runir::kr::dl::CategoryTag Category>
using ConstructorOrNonTerminal = ::cista::offset::variant<urd::index<NonTerminal<Category>>, Constructor<Category>>;

using NonTerminalTypes = boost::mp11::mp_transform<NonTerminal, runir::kr::dl::CategoryTags>;

using DerivationRuleTypes = boost::mp11::mp_transform<DerivationRule, runir::kr::dl::CategoryTags>;

using GrammarTypes = boost::mp11::mp_list<GrammarTag>;

using ConstructorRepositoryTypes = boost::mp11::mp_append<ConceptTypes,
                                                         RoleTypes,
                                                         BooleanTypes,
                                                         NumericalTypes,
                                                         NonTerminalTypes,
                                                         DerivationRuleTypes,
                                                         GrammarTypes>;

using ConstructorRepository = runir::kr::dl::detail::BasicConstructorRepository<ConstructorRepositoryTypes>;

using ConstructorRepositoryPtr = std::shared_ptr<ConstructorRepository>;

using ConstructorRepositoryFactory = runir::kr::dl::detail::BasicConstructorRepositoryFactory<ConstructorRepositoryTypes>;

template<typename Tag>
    requires runir::kr::dl::ConceptConstructorTag<Tag>
using ConceptView = urd::index_view<Concept<Tag>, ConstructorRepository>;

template<typename Tag>
    requires runir::kr::dl::RoleConstructorTag<Tag>
using RoleView = urd::index_view<Role<Tag>, ConstructorRepository>;

template<typename Tag>
    requires runir::kr::dl::BooleanConstructorTag<Tag>
using BooleanView = urd::index_view<Boolean<Tag>, ConstructorRepository>;

template<typename Tag>
    requires runir::kr::dl::NumericalConstructorTag<Tag>
using NumericalView = urd::index_view<Numerical<Tag>, ConstructorRepository>;

template<typename T>
using EntityView = urd::index_view<T, ConstructorRepository>;

/// A constructor read through the repository: the view of the record it holds.
template<runir::kr::dl::CategoryTag Category>
using ConstructorView = boost::mp11::mp_apply<std::variant, boost::mp11::mp_transform<EntityView, CategoryTypes<Category>>>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalView = urd::index_view<NonTerminal<Category>, ConstructorRepository>;

/// A grammar symbol read through the repository: its non-terminal's or its constructor's view.
template<runir::kr::dl::CategoryTag Category>
using ConstructorOrNonTerminalView = std::variant<NonTerminalView<Category>, ConstructorView<Category>>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleView = urd::index_view<DerivationRule<Category>, ConstructorRepository>;

using GrammarView = urd::index_view<GrammarTag, ConstructorRepository>;

}  // namespace runir::kr::dl::grammar

#endif
