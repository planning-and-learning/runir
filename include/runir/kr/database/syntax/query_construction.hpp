/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_SYNTAX_QUERY_CONSTRUCTION_HPP_
#define RUNIR_KR_DATABASE_SYNTAX_QUERY_CONSTRUCTION_HPP_

#include "runir/kr/database/semantics/plan_construction.hpp"
#include "runir/kr/database/syntax/query_builder.hpp"
#include "runir/kr/database/syntax/query_repository.hpp"
#include "yggdrasil/formalism/builder.hpp"
#include "yggdrasil/formalism/interning.hpp"

#include <algorithm>
#include <concepts>
#include <span>
#include <stdexcept>
#include <vector>

namespace runir::kr::database
{
/// Its builder types under a name of their own (yggdrasil's NamesList).
struct QueryBuilders : boost::mp11::mp_apply<::ygg::formalism::Builders, QuerySymbolTypes>
{
};

using QueryBuilder = ::ygg::formalism::BasicBuilderStorage<QueryRepository, QueryBuilders>;

using ::ygg::formalism::checkout;

namespace detail
{
std::span<const urd::data<ygg::ColumnLayout>> child_columns(urd::index<Query<>> child, const QueryRepository& repository);

/// The derived field of a query (its plan, or a rename's columns) from its other fields and its children's columns:
/// what its creator computes before it is interned. Inputs and empty relations derive nothing.
template<typename Tag>
    requires(std::same_as<Tag, QueryInputTag> || std::same_as<Tag, QueryEmptyTag>)
void derive(urd::data<Query<Tag>>& /*data*/, const QueryRepository& /*repository*/)
{
}

template<typename Tag>
    requires boost::mp11::mp_contains<QueryJoinTags, Tag>::value
void derive(urd::data<Query<Tag>>& data, const QueryRepository& repository)
{ data.plan = make_join_plan(child_columns(data.left_operand, repository), child_columns(data.right_operand, repository)); }

template<typename Tag>
    requires boost::mp11::mp_contains<QuerySetTags, Tag>::value
void derive(urd::data<Query<Tag>>& data, const QueryRepository& repository)
{ data.plan = make_set_plan(child_columns(data.left_operand, repository), child_columns(data.right_operand, repository)); }

void derive(urd::data<Query<QueryProjectTag>>& data, const QueryRepository& repository);

void derive(urd::data<Query<QueryRenameTag>>& data, const QueryRepository& repository);

void derive(urd::data<Query<QuerySelectEqualTag>>& data, const QueryRepository& repository);

void derive(urd::data<Query<QuerySelectValueTag>>& data, const QueryRepository& repository);

/// An omitted output order is canonicalized to first occurrence order.
void derive(urd::data<Query<QueryGenericJoinTag>>& data, const QueryRepository& repository);
}  // namespace detail

/// A query is prepared from its children and other fields, once they are its record's: its derived field (a plan, a
/// rename's columns) computed from them (detail::derive), its columns validated (an input's, an empty relation's).
template<typename Tag>
    requires(!std::same_as<Tag, void>)
void prepare_for_insert(const QueryRepository& repository, urd::data<Query<Tag>>& data)
{
    detail::derive(data, repository);
    if constexpr (requires { data.columns; } && !std::same_as<Tag, QueryRenameTag>)
        ygg::validate_columns({ data.columns.data(), data.columns.size() });
}

/// The concrete record must already be interned.
void prepare_for_insert(const QueryRepository& repository, urd::data<Query<>>& data);

/// Interns a built query (prepared: prepare_for_insert), then its heterogeneous root.
template<typename Tag>
QueryView<> insert_query(QueryRepository& repository, QueryBuilder& builder, const urd::builder<Query<Tag>, QueryRepository>& built)
{
    auto root = checkout<Query<>>(builder);
    urd::store(root->variant, repository.insert(built, builder).first);
    return repository.insert(*root, builder).first;
}
}  // namespace runir::kr::database

#endif
