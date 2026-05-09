#ifndef RUNIR_CNF_GRAMMAR_DATA_HELPERS_HPP_
#define RUNIR_CNF_GRAMMAR_DATA_HELPERS_HPP_

#include "runir/knowledge_representation/dl/cnf_grammar/indices.hpp"
#include "runir/knowledge_representation/dl/semantics/data_helpers.hpp"

#include <tyr/common/types.hpp>

namespace runir::kr::dl::cnf_grammar
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

}

#endif
