/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "runir/kr/database/datalog/repository.hpp"

#include "runir/kr/database/datalog/builder.hpp"
#include "runir/kr/database/datalog/construction.hpp"

#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::kr::database::datalog
{
// Numbered as yggdrasil's layers are, from the one process-wide counter, so that no two contexts share a number.
DatalogRepository::DatalogRepository() : m_index(ygg::next_layer()) {}

DatalogRepository::~DatalogRepository() = default;
}


// The repository's symbols, which the library's other sources only declare (yggdrasil/formalism/symbol_context.hpp).
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::kr::database::datalog::DatalogRepository, 5)
YGG_FORMALISM_BUILDER(, runir::kr::database::datalog::DatalogBuilder, 5)
