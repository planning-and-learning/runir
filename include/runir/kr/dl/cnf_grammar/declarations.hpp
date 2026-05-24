#ifndef RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_
#define RUNIR_CNF_GRAMMAR_DECLARATIONS_HPP_

#include "runir/kr/dl/declarations.hpp"

namespace runir::kr::dl::cnf_grammar
{

template<runir::kr::dl::FamilyTag Family>
struct Grammar;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyConceptConstructorTag<Family, Tag>
struct Concept;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyRoleConstructorTag<Family, Tag>
struct Role;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyBooleanConstructorTag<Family, Tag>
struct Boolean;

template<runir::kr::dl::FamilyTag Family, typename Tag>
    requires runir::kr::dl::FamilyNumericalConstructorTag<Family, Tag>
struct Numerical;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct Constructor;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct NonTerminal;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct DerivationRule;

template<runir::kr::dl::FamilyTag Family, runir::kr::dl::CategoryTag Category>
struct SubstitutionRule;

}

#endif
