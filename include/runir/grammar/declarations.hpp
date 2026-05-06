#ifndef RUNIR_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_GRAMMAR_DECLARATIONS_HPP_

#include "runir/declarations.hpp"

namespace runir::grammar
{

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
struct ConstructorOrNonTerminal;

template<runir::CategoryTag Category>
struct DerivationRule;

}

#endif
