#ifndef RUNIR_CNF_GRAMMAR_DATA_HELPERS_HPP_
#define RUNIR_CNF_GRAMMAR_DATA_HELPERS_HPP_

#include "runir/kr/dl/cnf_grammar/declarations.hpp"
#include "runir/kr/dl/data_helpers.hpp"

#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::cnf_grammar
{

template<runir::kr::dl::FamilyTag Family, typename Self>
using NumberRestrictionData = runir::kr::dl::NumberRestrictionData<Self, NonTerminal<Family, RoleTag>>;

template<runir::kr::dl::FamilyTag Family, typename Self>
using QualifiedNumberRestrictionData = runir::kr::dl::QualifiedNumberRestrictionData<Self, NonTerminal<Family, RoleTag>, NonTerminal<Family, ConceptTag>>;

template<runir::kr::dl::FamilyTag Family, typename Self>
using RoleFillersData = runir::kr::dl::RoleFillersData<Self, NonTerminal<Family, RoleTag>>;

}

#endif
