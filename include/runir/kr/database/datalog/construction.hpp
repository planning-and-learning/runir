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
/// Its builder types under a name of their own (yggdrasil's NamesList).
struct DatalogBuilders : boost::mp11::mp_apply<::ygg::formalism::Builders, DatalogSymbolTypes>
{
};

using DatalogBuilder = ::ygg::formalism::BasicBuilderStorage<DatalogRepository, DatalogBuilders>;

using ::ygg::formalism::checkout;

void prepare_for_insert(const DatalogRepository& repository, urd::data<Relation>& data);

/// Requires as many terms as the relation's arity.
void prepare_for_insert(const DatalogRepository& repository, urd::data<Atom>& data);

/// Requires every variable of the negated atoms and comparisons, and the value of SUM, MIN and MAX, in a positive atom of
/// the aggregate, and a result that its atoms do not use.
void prepare_for_insert(const DatalogRepository& repository, urd::data<Aggregate>& data);

/// Requires a derived head with distinct variables, and every variable of the head, the negated atoms and the
/// comparisons in a positive atom or as an aggregate's result; the results are distinct and not in a positive atom.
void prepare_for_insert(const DatalogRepository& repository, urd::data<Rule>& data);

}  // namespace runir::kr::database::datalog

#endif
