#ifndef RUNIR_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_GRAMMAR_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

namespace runir::kr::dl::grammar
{

struct GrammarTag;

template<runir::kr::dl::ConceptConstructorTag Tag>
struct Concept;

template<runir::kr::dl::RoleConstructorTag Tag>
struct Role;

template<runir::kr::dl::BooleanConstructorTag Tag>
struct Boolean;

template<runir::kr::dl::NumericalConstructorTag Tag>
struct Numerical;

template<runir::kr::dl::CategoryTag Category>
struct Constructor;

template<runir::kr::dl::CategoryTag Category>
struct NonTerminal;

template<runir::kr::dl::CategoryTag Category>
struct ConstructorOrNonTerminal;

template<runir::kr::dl::CategoryTag Category>
struct DerivationRule;

}

#endif
