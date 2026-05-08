#ifndef RUNIR_GRAMMAR_DATA_HELPERS_HPP_
#define RUNIR_GRAMMAR_DATA_HELPERS_HPP_

#include "runir/grammar/indices.hpp"
#include "runir/semantics/data_helpers.hpp"

#include <tyr/common/types.hpp>

namespace runir::grammar
{

template<typename Self>
using NullaryData = runir::semantics::NullaryData<Self>;

template<typename Self, typename Arg>
using UnaryData = runir::semantics::UnaryData<Self, Arg>;

template<typename Self, typename Lhs, typename Rhs>
using BinaryData = runir::semantics::BinaryData<Self, Lhs, Rhs>;

template<typename Self, typename Lhs, typename Mid, typename Rhs>
using TernaryData = runir::semantics::TernaryData<Self, Lhs, Mid, Rhs>;

template<typename Self, tyr::formalism::FactKind T>
using PredicateData = runir::semantics::PredicateData<Self, T>;

template<typename Self>
using ObjectData = runir::semantics::ObjectData<Self>;

}

#endif
