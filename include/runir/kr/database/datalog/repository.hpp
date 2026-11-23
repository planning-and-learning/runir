/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_DATALOG_REPOSITORY_HPP_
#define RUNIR_KR_DATABASE_DATALOG_REPOSITORY_HPP_

#include "runir/kr/database/datalog/view.hpp"
#include "yggdrasil/formalism/declarations.hpp"
#include "yggdrasil/formalism/symbol_context.hpp"

#include <cstddef>
#include <urd/config.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::kr::database::datalog
{
/// Interns Datalog records. Records are prepared by prepare_for_insert (construction.hpp) before insertion; children
/// precede their parents. Each repository has its own number.
class DatalogRepository : public ::ygg::formalism::SymbolContext<DatalogRepository, DatalogSymbolTypes>
{
    size_t m_index;

public:
    DatalogRepository();
    ~DatalogRepository();
    DatalogRepository(const DatalogRepository&) = delete;
    DatalogRepository& operator=(const DatalogRepository&) = delete;
    DatalogRepository(DatalogRepository&&) = delete;
    DatalogRepository& operator=(DatalogRepository&&) = delete;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }
};
}  // namespace runir::kr::database::datalog

// Its symbols' reads, finds and inserts instantiated once, where the library instantiates the repository (brokkr's
// include rule I7): a file that reads records through it completes none of its stores.
YGG_FORMALISM_SYMBOL_CONTEXT(extern, runir::kr::database::datalog::DatalogRepository, 5)

#endif
