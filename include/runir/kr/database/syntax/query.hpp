/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_SYNTAX_QUERY_HPP_
#define RUNIR_KR_DATABASE_SYNTAX_QUERY_HPP_

#include "runir/kr/database/syntax/query_construction.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <urd/config.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace runir::kr::database
{
/// Whether the query's operation has the tag.
template<class Tag>
bool is(QueryView<> query)
{ return std::holds_alternative<QueryView<Tag>>(query.variant()); }

/// The query's operation; requires is<Tag>(query).
template<class Tag>
QueryView<Tag> as(QueryView<> query)
{ return std::get<QueryView<Tag>>(query.variant()); }

namespace detail
{
/// Rebinds a query's operation to schema-identical children in the destination.
QueryView<> clone_query(QueryView<> source, std::span<const QueryView<>> children, QueryRepository& destination, QueryBuilder& builder);
}  // namespace detail

/// An immutable owning snapshot of the same typed query records used to build queries.
/// Copies share its stable repository; repository indices are topological, while roots retain caller order.
class QueryPlan
{
    using QueryIndex = urd::index<Query<>>;
    std::shared_ptr<const QueryRepository> m_repository;
    urd::Store<urd::sequence<QueryIndex>> m_roots;

public:
    QueryPlan() : m_repository(std::make_shared<QueryRepository>()) {}

    QueryPlan(std::shared_ptr<const QueryRepository> repository, urd::Store<urd::sequence<QueryIndex>> roots) :
        m_repository(std::move(repository)),
        m_roots(std::move(roots))
    {
    }

    [[nodiscard]] size_t node_count() const noexcept { return m_repository->size(); }

    [[nodiscard]] size_t root_count() const noexcept { return m_roots.size(); }

    [[nodiscard]] std::span<const QueryIndex> roots() const& noexcept { return m_roots; }

    [[nodiscard]] std::span<const QueryIndex> roots() const&& = delete;

    [[nodiscard]] const QueryRepository& get_repository() const& noexcept { return *m_repository; }

    [[nodiscard]] const QueryRepository& get_repository() const&& = delete;

    QueryView<> operator[](QueryIndex id) const&
    {
        if (id.value >= node_count())
            throw std::out_of_range("Query plan: invalid query index.");
        return m_repository->view(id);
    }

    QueryView<> operator[](QueryIndex id) const&& = delete;
};

/// The queries reachable from roots of one repository, each once, children first.
urd::Store<urd::sequence<QueryView<>>> reachable(std::span<const QueryView<>> roots);

}  // namespace runir::kr::database

namespace runir::kr::database
{

/// The input slots read by the queries reachable from `roots`, each once, in increasing order.
urd::Store<urd::sequence<size_t>> input_slots(std::span<const QueryView<>> roots);

/// Copies the queries reachable from the roots into a fresh owning snapshot.
QueryPlan compile_plan(std::span<const QueryView<>> roots);

template<std::ranges::contiguous_range R>
auto compile_plan(const R& roots)
{ return compile_plan(std::span<const std::ranges::range_value_t<R>>(roots)); }

QueryPlan compile_plan(std::initializer_list<QueryView<>> roots);
}  // namespace runir::kr::database

#endif
