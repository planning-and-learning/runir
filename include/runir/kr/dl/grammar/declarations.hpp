#ifndef RUNIR_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_GRAMMAR_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <boost/mp11.hpp>
#include <cista/containers/variant.h>
#include <concepts>
#include <memory>
#include <type_traits>
#include <urd/elements/flat.hpp>
#include <variant>

namespace runir::kr::dl::grammar
{

template<runir::kr::dl::CategoryTag Category>
struct NonTerminal;

template<runir::kr::dl::CategoryTag Category>
struct DerivationRule;

struct GrammarTag;

struct GrammarFamily;

/// Grammar constructors: the base constructor tags, operands stored as grammar symbols (a non-terminal or a constructor).
/// The records name the family's tags, so the symbol type is spelled here, after Tags, rather than before the family.
struct GrammarFamily
{
    template<runir::kr::dl::CategoryTag Category>
    using Tags = runir::kr::dl::BaseConstructorTags<Category>;

    template<runir::kr::dl::CategoryTag Category>
    using Operand = urd::flat_variant<urd::index<NonTerminal<Category>>, urd::index_variant<runir::kr::dl::RecordTypes<GrammarFamily, Category>>>;

    using Argument = urd::flat_variant<Operand<runir::kr::dl::ConceptTag>, Operand<runir::kr::dl::RoleTag>>;
    using Vertex = Operand<runir::kr::dl::ConceptTag>;
    using Edge = Operand<runir::kr::dl::RoleTag>;
};

/// A constructor: one of Category's records. Stored as it is, not interned.
template<runir::kr::dl::CategoryTag Category>
using Constructor = urd::index_variant<runir::kr::dl::RecordTypes<GrammarFamily, Category>>;

/// A grammar symbol of Category: a non-terminal or a constructor. Stored as it is, not interned.
template<runir::kr::dl::CategoryTag Category>
using ConstructorOrNonTerminal = GrammarFamily::Operand<Category>;

using ConceptTypes = runir::kr::dl::RecordTypes<GrammarFamily, runir::kr::dl::ConceptTag>;

using RoleTypes = runir::kr::dl::RecordTypes<GrammarFamily, runir::kr::dl::RoleTag>;

using BooleanTypes = runir::kr::dl::RecordTypes<GrammarFamily, runir::kr::dl::BooleanTag>;

using NumericalTypes = runir::kr::dl::RecordTypes<GrammarFamily, runir::kr::dl::NumericalTag>;

using NonTerminalTypes = boost::mp11::mp_transform<NonTerminal, runir::kr::dl::CategoryTags>;

using DerivationRuleTypes = boost::mp11::mp_transform<DerivationRule, runir::kr::dl::CategoryTags>;

using GrammarTypes = boost::mp11::mp_list<GrammarTag>;

using ConstructorRepositoryTypes =
    boost::mp11::mp_append<ConceptTypes, RoleTypes, BooleanTypes, NumericalTypes, NonTerminalTypes, DerivationRuleTypes, GrammarTypes>;

using ConstructorRepository = runir::kr::dl::detail::BasicConstructorRepository<ConstructorRepositoryTypes>;

using ConstructorRepositoryPtr = std::shared_ptr<ConstructorRepository>;

using ConstructorRepositoryFactory = runir::kr::dl::detail::BasicConstructorRepositoryFactory<ConstructorRepositoryTypes>;

template<runir::kr::dl::ConstructorOf<GrammarFamily, runir::kr::dl::ConceptTag> Tag>
using ConceptView = urd::index_view<runir::kr::dl::Concept<GrammarFamily, Tag>, ConstructorRepository>;

template<runir::kr::dl::ConstructorOf<GrammarFamily, runir::kr::dl::RoleTag> Tag>
using RoleView = urd::index_view<runir::kr::dl::Role<GrammarFamily, Tag>, ConstructorRepository>;

template<runir::kr::dl::ConstructorOf<GrammarFamily, runir::kr::dl::BooleanTag> Tag>
using BooleanView = urd::index_view<runir::kr::dl::Boolean<GrammarFamily, Tag>, ConstructorRepository>;

template<runir::kr::dl::ConstructorOf<GrammarFamily, runir::kr::dl::NumericalTag> Tag>
using NumericalView = urd::index_view<runir::kr::dl::Numerical<GrammarFamily, Tag>, ConstructorRepository>;

template<typename T>
using EntityView = urd::index_view<T, ConstructorRepository>;

/// A constructor read through the repository: the view of the record it holds.
template<runir::kr::dl::CategoryTag Category>
using ConstructorView = boost::mp11::mp_apply<std::variant, boost::mp11::mp_transform<EntityView, runir::kr::dl::RecordTypes<GrammarFamily, Category>>>;

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
