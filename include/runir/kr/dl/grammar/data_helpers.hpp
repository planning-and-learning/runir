#ifndef RUNIR_GRAMMAR_DATA_HELPERS_HPP_
#define RUNIR_GRAMMAR_DATA_HELPERS_HPP_

#include "runir/kr/dl/data_helpers.hpp"
#include "runir/kr/dl/grammar/declarations.hpp"

#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::grammar
{

template<typename Self>
using NumberRestrictionData = runir::kr::dl::NumberRestrictionData<Self, ConstructorOrNonTerminal<RoleTag>>;

template<typename Self>
using QualifiedNumberRestrictionData =
    runir::kr::dl::QualifiedNumberRestrictionData<Self, ConstructorOrNonTerminal<RoleTag>, ConstructorOrNonTerminal<ConceptTag>>;

template<typename Self>
using RoleFillersData = runir::kr::dl::RoleFillersData<Self, ConstructorOrNonTerminal<RoleTag>>;
}

#endif
