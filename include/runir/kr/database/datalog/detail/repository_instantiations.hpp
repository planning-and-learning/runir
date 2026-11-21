/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_DATALOG_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define RUNIR_KR_DATABASE_DATALOG_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// The interning of every Datalog record type, instantiated in the library (datalog/repository.cpp). Include only from
/// sources that intern many of these types; see rule 9 of the template rules
/// (https://planning-and-learning.github.io/brokkr/template_rules.html).

#include "runir/kr/database/datalog/repository.hpp"
#include "yggdrasil/formalism/intern_instantiations.hpp"

/// The instantiations (PREFIX extern: declarations).
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage): it expands to explicit instantiations, declarations, which no function gives
#define RUNIR_KR_DATABASE_DATALOG_PROGRAM_INTERNS(PREFIX) YGG_FORMALISM_INTERNS(PREFIX, 5, false, runir::kr::database::datalog::DatalogSymbolTypes)

RUNIR_KR_DATABASE_DATALOG_PROGRAM_INTERNS(extern)

#endif
