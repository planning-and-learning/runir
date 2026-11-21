/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef RUNIR_KR_DATABASE_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_
#define RUNIR_KR_DATABASE_DETAIL_REPOSITORY_INSTANTIATIONS_HPP_

/// Extern declarations of intern for every symbol type of the database's repositories (queries, relations),
/// instantiated in the library (syntax/query_repository.cpp). Include only from sources that intern many of these
/// types; see rule 9 of the template rules
/// (https://planning-and-learning.github.io/brokkr/template_rules.html).

#include "runir/kr/database/semantics/relation_view.hpp"
#include "runir/kr/database/syntax/query_repository.hpp"
#include "yggdrasil/formalism/intern_instantiations.hpp"

/// The instantiations (PREFIX extern: declarations).
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage): it expands to explicit instantiations, declarations, which no function gives
#define RUNIR_KR_DATABASE_INTERNS(PREFIX) \
    YGG_FORMALISM_INTERNS(PREFIX, 15, false, boost::mp11::mp_append<runir::kr::database::QuerySymbolTypes, ygg::FreeRelationRepository::SymbolTypes>)

RUNIR_KR_DATABASE_INTERNS(extern)

#endif
