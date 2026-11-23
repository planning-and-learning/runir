/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_SYNTAX_QUERY_REPOSITORY_HPP_
#define RUNIR_KR_DATABASE_SYNTAX_QUERY_REPOSITORY_HPP_

#include "runir/kr/database/syntax/query_view.hpp"
#include "yggdrasil/formalism/declarations.hpp"
#include "yggdrasil/formalism/symbol_context.hpp"

#include <cassert>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <urd/config.hpp>
#include <utility>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::kr::database
{
/// Interns typed query records. Records are prepared by prepare_for_insert
/// The erased query record followed by one concrete record per constructor tag.
using QuerySymbolTypes = boost::mp11::mp_append<boost::mp11::mp_list<Query<>>, boost::mp11::mp_transform<Query, QueryConstructorTags>>;

/// (query_construction.hpp) before insertion; children precede their parents. Each repository has its own number.
class QueryRepository : public ::ygg::formalism::SymbolContext<QueryRepository, QuerySymbolTypes>
{
    using Base = ::ygg::formalism::SymbolContext<QueryRepository, QuerySymbolTypes>;

    size_t m_index;

public:
    QueryRepository();
    ~QueryRepository();

    /// An index as its view here (urd's resolves contract): what compositions of indices are read through (urd::resolve).
    template<typename T>
    [[nodiscard]] urd::index_view<T, QueryRepository> view(urd::index<T> index) const noexcept
    { return { index, this }; }

    QueryRepository(const QueryRepository&) = delete;
    QueryRepository& operator=(const QueryRepository&) = delete;
    QueryRepository(QueryRepository&&) = delete;
    QueryRepository& operator=(QueryRepository&&) = delete;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }

    /// Number of root queries. Root indices are topological.
    [[nodiscard]] size_t size() const { return Base::template size<Query<>>(); }

    using Base::size;
};

}  // namespace runir::kr::database

// Its symbols' reads, finds and inserts instantiated once, where the library instantiates the repository (brokkr's
// include rule I7): a file that reads records through it completes none of its stores.
YGG_FORMALISM_SYMBOL_CONTEXT(extern, runir::kr::database::QueryRepository, 13)

#endif
