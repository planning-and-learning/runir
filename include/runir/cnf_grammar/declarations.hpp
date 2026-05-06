#ifndef RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_

#include "runir/declarations.hpp"

namespace runir::cnf_grammar
{

struct GrammarTag;

template<runir::ConceptConstructorTag Tag>
struct Concept;

template<runir::RoleConstructorTag Tag>
struct Role;

template<runir::BooleanConstructorTag Tag>
struct Boolean;

template<runir::NumericalConstructorTag Tag>
struct Numerical;

template<runir::CategoryTag Category>
struct Constructor;

template<runir::CategoryTag Category>
struct NonTerminal;

template<runir::CategoryTag Category>
struct DerivationRule;

template<runir::CategoryTag Category>
struct SubstitutionRule;

}

#endif
