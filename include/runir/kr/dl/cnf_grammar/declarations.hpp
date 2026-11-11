#ifndef RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <boost/mp11.hpp>
#include <concepts>
#include <memory>
#include <type_traits>
#include <urd/elements/flat.hpp>
#include <variant>

namespace runir::kr::dl::cnf_grammar
{

struct Grammar;

template<runir::kr::dl::CategoryTag Category>
struct NonTerminal;

template<runir::kr::dl::CategoryTag Category>
struct DerivationRule;

template<runir::kr::dl::CategoryTag Category>
struct SubstitutionRule;

struct CnfGrammarFamily;

/// CNF grammar constructors: the base constructor tags, operands non-terminals.
struct CnfGrammarFamily
{
    template<runir::kr::dl::CategoryTag Category>
    using Tags = runir::kr::dl::BaseConstructorTags<Category>;

    template<runir::kr::dl::CategoryTag Category>
    using Operand = urd::index<NonTerminal<Category>>;

    using Argument = urd::flat_variant<Operand<runir::kr::dl::ConceptTag>, Operand<runir::kr::dl::RoleTag>>;
    using Vertex = Operand<runir::kr::dl::ConceptTag>;
    using Edge = Operand<runir::kr::dl::RoleTag>;
};

/// A constructor: one of Category's records. Stored as it is, not interned.
template<runir::kr::dl::CategoryTag Category>
using Constructor = urd::index_variant<runir::kr::dl::RecordTypes<CnfGrammarFamily, Category>>;

using ConceptTypes = runir::kr::dl::RecordTypes<CnfGrammarFamily, runir::kr::dl::ConceptTag>;

using RoleTypes = runir::kr::dl::RecordTypes<CnfGrammarFamily, runir::kr::dl::RoleTag>;

using BooleanTypes = runir::kr::dl::RecordTypes<CnfGrammarFamily, runir::kr::dl::BooleanTag>;

using NumericalTypes = runir::kr::dl::RecordTypes<CnfGrammarFamily, runir::kr::dl::NumericalTag>;

using NonTerminalTypes = boost::mp11::mp_transform<NonTerminal, runir::kr::dl::CategoryTags>;

using DerivationRuleTypes = boost::mp11::mp_transform<DerivationRule, runir::kr::dl::CategoryTags>;

using SubstitutionRuleTypes = boost::mp11::mp_transform<SubstitutionRule, runir::kr::dl::CategoryTags>;

using GrammarTypes = boost::mp11::mp_list<Grammar>;

using ConstructorRepositoryTypes =
    boost::mp11::mp_append<ConceptTypes, RoleTypes, BooleanTypes, NumericalTypes, NonTerminalTypes, DerivationRuleTypes, SubstitutionRuleTypes, GrammarTypes>;

using ConstructorRepository = runir::kr::dl::detail::BasicConstructorRepository<ConstructorRepositoryTypes>;

using ConstructorRepositoryPtr = std::shared_ptr<ConstructorRepository>;

using ConstructorRepositoryFactory = runir::kr::dl::detail::BasicConstructorRepositoryFactory<ConstructorRepositoryTypes>;

template<runir::kr::dl::ConstructorOf<CnfGrammarFamily, runir::kr::dl::ConceptTag> Tag>
using ConceptView = urd::index_view<runir::kr::dl::Concept<CnfGrammarFamily, Tag>, ConstructorRepository>;

template<runir::kr::dl::ConstructorOf<CnfGrammarFamily, runir::kr::dl::RoleTag> Tag>
using RoleView = urd::index_view<runir::kr::dl::Role<CnfGrammarFamily, Tag>, ConstructorRepository>;

template<runir::kr::dl::ConstructorOf<CnfGrammarFamily, runir::kr::dl::BooleanTag> Tag>
using BooleanView = urd::index_view<runir::kr::dl::Boolean<CnfGrammarFamily, Tag>, ConstructorRepository>;

template<runir::kr::dl::ConstructorOf<CnfGrammarFamily, runir::kr::dl::NumericalTag> Tag>
using NumericalView = urd::index_view<runir::kr::dl::Numerical<CnfGrammarFamily, Tag>, ConstructorRepository>;

template<typename T>
using EntityView = urd::index_view<T, ConstructorRepository>;

/// A constructor read through the repository: the view of the record it holds.
template<runir::kr::dl::CategoryTag Category>
using ConstructorView = boost::mp11::mp_apply<std::variant, boost::mp11::mp_transform<EntityView, runir::kr::dl::RecordTypes<CnfGrammarFamily, Category>>>;

template<runir::kr::dl::CategoryTag Category>
using NonTerminalView = urd::index_view<NonTerminal<Category>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using DerivationRuleView = urd::index_view<DerivationRule<Category>, ConstructorRepository>;

template<runir::kr::dl::CategoryTag Category>
using SubstitutionRuleView = urd::index_view<SubstitutionRule<Category>, ConstructorRepository>;

using GrammarView = urd::index_view<Grammar, ConstructorRepository>;
}

#endif
