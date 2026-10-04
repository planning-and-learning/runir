#ifndef RUNIR_GRAMMAR_DATA_HELPERS_HPP_
#define RUNIR_GRAMMAR_DATA_HELPERS_HPP_

#include "runir/kr/dl/data_helpers.hpp"
#include "runir/kr/dl/grammar/indices.hpp"

#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::grammar
{

template<runir::kr::dl::FamilyTag Family, typename Self>
using NumberRestrictionData = runir::kr::dl::NumberRestrictionData<Self, ConstructorOrNonTerminal<Family, RoleTag>>;

template<runir::kr::dl::FamilyTag Family, typename Self>
using QualifiedNumberRestrictionData =
    runir::kr::dl::QualifiedNumberRestrictionData<Self, ConstructorOrNonTerminal<Family, RoleTag>, ConstructorOrNonTerminal<Family, ConceptTag>>;

template<runir::kr::dl::FamilyTag Family, typename Self>
using RoleFillersData = runir::kr::dl::RoleFillersData<Self, ConstructorOrNonTerminal<Family, RoleTag>>;

}

#endif
