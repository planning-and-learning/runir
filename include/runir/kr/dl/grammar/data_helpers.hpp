#ifndef RUNIR_GRAMMAR_DATA_HELPERS_HPP_
#define RUNIR_GRAMMAR_DATA_HELPERS_HPP_

#include "runir/kr/dl/grammar/indices.hpp"
#include "runir/kr/dl/semantics/data_helpers.hpp"

#include <yggdrasil/core/types.hpp>

namespace runir::kr::dl::grammar
{

template<typename Self>
using NullaryData = runir::kr::dl::semantics::NullaryData<Self>;

template<typename Self, typename Arg>
using UnaryData = runir::kr::dl::semantics::UnaryData<Self, Arg>;

template<typename Self, typename Lhs, typename Rhs>
using BinaryData = runir::kr::dl::semantics::BinaryData<Self, Lhs, Rhs>;

template<typename Self, typename Lhs, typename Mid, typename Rhs>
using TernaryData = runir::kr::dl::semantics::TernaryData<Self, Lhs, Mid, Rhs>;

template<typename Self, tyr::formalism::FactKind T>
using PredicateData = runir::kr::dl::semantics::PredicateData<Self, T>;

template<typename Self>
using ObjectData = runir::kr::dl::semantics::ObjectData<Self>;

template<runir::kr::dl::FamilyTag Family, typename Self>
using NumberRestrictionData = runir::kr::dl::semantics::NumberRestrictionData<Self, ConstructorOrNonTerminal<Family, RoleTag>>;

template<runir::kr::dl::FamilyTag Family, typename Self>
using QualifiedNumberRestrictionData =
    runir::kr::dl::semantics::QualifiedNumberRestrictionData<Self, ConstructorOrNonTerminal<Family, RoleTag>, ConstructorOrNonTerminal<Family, ConceptTag>>;

template<runir::kr::dl::FamilyTag Family, typename Self>
using RoleFillersData = runir::kr::dl::semantics::RoleFillersData<Self, ConstructorOrNonTerminal<Family, RoleTag>>;

template<typename Self>
using ObjectListData = runir::kr::dl::semantics::ObjectListData<Self>;

}

#endif
