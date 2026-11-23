/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "runir/kr/database/syntax/query_construction.hpp"

#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::database::detail
{
std::span<const urd::data<ygg::ColumnLayout>> child_columns(urd::index<Query<>> child, const QueryRepository& repository)
{
    if (child.value >= repository.size())
        throw std::invalid_argument("Query: children must precede their parent.");
    return columns_of(repository.view(child));
}
}

namespace runir::kr::database::detail
{
void derive(urd::data<Query<QueryProjectTag>>& data, const QueryRepository& repository)
{ data.plan = make_projection_plan(child_columns(data.operand, repository), { data.labels.data(), data.labels.size() }); }
}

namespace runir::kr::database::detail
{
void derive(urd::data<Query<QueryRenameTag>>& data, const QueryRepository& repository)
{
    const auto schema = child_columns(data.operand, repository);
    if (data.labels.size() != schema.size())
        throw std::invalid_argument("Columns: renaming must preserve arity.");
    data.columns.set(schema.begin(), schema.end());
    for (size_t i = 0; i < schema.size(); ++i)
        data.columns[i].label = data.labels[i];
    ygg::validate_columns({ data.columns.data(), data.columns.size() });
}
}

namespace runir::kr::database::detail
{
void derive(urd::data<Query<QuerySelectEqualTag>>& data, const QueryRepository& repository)
{ data.plan = make_select_equal_plan(child_columns(data.operand, repository), data.left_column, data.right_column); }
}

namespace runir::kr::database::detail
{
void derive(urd::data<Query<QuerySelectValueTag>>& data, const QueryRepository& repository)
{ data.plan = make_select_value_plan(child_columns(data.operand, repository), data.column, data.constant_type, data.constant); }
}

namespace runir::kr::database::detail
{
void derive(urd::data<Query<QueryGenericJoinTag>>& data, const QueryRepository& repository)
{
    urd::Store<urd::sequence<urd::Store<urd::sequence<urd::data<ygg::ColumnLayout>>>>> schemas;
    for (const auto child : data.inputs)
    {
        const auto schema = child_columns(child, repository);
        schemas.emplace_back(schema.begin(), schema.end());
    }
    if (data.output_order.empty())
        for (const auto& schema : schemas)
            for (const auto& column : schema)
                if (std::ranges::find(data.output_order, column.label) == data.output_order.end())
                    data.output_order.push_back(column.label);
    data.plan =
        make_generic_join_plan(schemas, { data.variable_order.data(), data.variable_order.size() }, { data.output_order.data(), data.output_order.size() });
}
}

namespace runir::kr::database
{
void prepare_for_insert(const QueryRepository& repository, urd::data<Query<>>& data)
{
    data.variant.apply(
        [&]<typename Tag>(urd::index<Query<Tag>> index)
        {
            if (index.value >= repository.template size<Query<Tag>>())
                throw std::invalid_argument("Query: root refers to a missing query.");
        });
}
}
