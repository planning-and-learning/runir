/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "runir/kr/database/syntax/query_repository.hpp"

#include "runir/kr/database/detail/repository_instantiations.hpp"
#include "runir/kr/database/syntax/query_builder.hpp"
#include "runir/kr/database/syntax/query_construction.hpp"

#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::kr::database
{
// Numbered as yggdrasil's layers are, from the one process-wide counter, so that no two contexts share a number.
QueryRepository::QueryRepository() : m_index(ygg::next_layer()) {}

QueryRepository::~QueryRepository() = default;
}

RUNIR_KR_DATABASE_INTERNS()

// The repository's symbols, which the library's other sources only declare (yggdrasil/formalism/symbol_context.hpp).
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::database::QueryRepository, 13)
YGG_FORMALISM_BUILDER(, runir::kr::database::QueryBuilder, 13)
