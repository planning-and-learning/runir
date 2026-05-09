#ifndef RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_

#include "runir/knowledge_representation/dl/declarations.hpp"

namespace runir::kr::dl::cnf_grammar
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
struct DerivationRule;

template<runir::kr::dl::CategoryTag Category>
struct SubstitutionRule;

}

#endif
