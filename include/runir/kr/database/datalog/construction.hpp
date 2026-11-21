/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_DATALOG_CONSTRUCTION_HPP_
#define RUNIR_KR_DATABASE_DATALOG_CONSTRUCTION_HPP_

#include "runir/kr/database/datalog/builder.hpp"
#include "runir/kr/database/datalog/repository.hpp"
#include "yggdrasil/formalism/builder.hpp"
#include "yggdrasil/formalism/interning.hpp"

#include <boost/mp11.hpp>

namespace runir::kr::database::datalog
{
using DatalogBuilder = boost::mp11::mp_apply<::ygg::formalism::BuilderStorage, boost::mp11::mp_push_front<DatalogSymbolTypes, DatalogRepository>>;

using ::ygg::formalism::checkout;
using ::ygg::formalism::insert;

void prepare_for_insert(DatalogRepository& repository, urd::data<Relation>& data);

/// Requires as many terms as the relation's arity.
void prepare_for_insert(DatalogRepository& repository, urd::data<Atom>& data);

/// Requires every variable of the negated atoms and comparisons, and the value of SUM, MIN and MAX, in a positive atom of
/// the aggregate, and a result that its atoms do not use.
void prepare_for_insert(DatalogRepository& repository, urd::data<Aggregate>& data);

/// Requires a derived head with distinct variables, and every variable of the head, the negated atoms and the
/// comparisons in a positive atom or as an aggregate's result; the results are distinct and not in a positive atom.
void prepare_for_insert(DatalogRepository& repository, urd::data<Rule>& data);

inline void prepare_for_insert(DatalogRepository&, urd::data<Program>&) {}
}  // namespace runir::kr::database::datalog

#endif
