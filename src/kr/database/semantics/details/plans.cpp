/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "runir/kr/database/semantics/plan_construction.hpp"

#include <algorithm>
#include <stdexcept>

namespace
{
/// The position of the column labeled `label`; std::out_of_range where there is none.
size_t column_position(std::span<const urd::data<ygg::ColumnLayout>> columns, urd::index<ygg::Column> label)
{
    const auto found = std::ranges::find(columns, label, &urd::data<ygg::ColumnLayout>::label);
    if (found == columns.end())
        throw std::out_of_range("Columns: unknown column label.");
    return static_cast<size_t>(found - columns.begin());
}
}  // namespace

namespace runir::kr::database
{
urd::data<ProjectionPlan> make_projection_plan(std::span<const urd::data<ygg::ColumnLayout>> input, std::span<const urd::index<ygg::Column>> columns)
{
    ygg::validate_columns(input);
    auto plan = urd::data<ProjectionPlan> {};
    plan.input_columns.set(input.begin(), input.end());
    for (const auto label : columns)
    {
        const auto position = column_position(input, label);
        plan.output_columns.push_back(input[position]);
        plan.positions.push_back(ColumnSlice(ygg::to_uint_t(position)));
    }
    ygg::validate_columns(plan.output_columns);
    return plan;
}

urd::data<JoinPlan> make_join_plan(std::span<const urd::data<ygg::ColumnLayout>> left, std::span<const urd::data<ygg::ColumnLayout>> right)
{
    ygg::validate_columns(left);
    ygg::validate_columns(right);
    auto plan = urd::data<JoinPlan> {};
    plan.left_columns.set(left.begin(), left.end());
    plan.right_columns.set(right.begin(), right.end());
    plan.output_columns.set(left.begin(), left.end());
    for (size_t position = 0; position < right.size(); ++position)
    {
        const auto& column = right[position];
        const auto it = std::ranges::find(left, column.label, &urd::data<ygg::ColumnLayout>::label);
        if (it == left.end())
        {
            plan.output_columns.push_back(column);
            plan.right_payload.push_back(ColumnSlice(ygg::to_uint_t(position)));
        }
        else
        {
            if (it->type != column.type)
                throw std::invalid_argument("Relational operation: common columns must have the same type.");
            plan.left_keys.push_back(ColumnSlice(ygg::to_uint_t(static_cast<size_t>(it - left.begin()))));
            plan.right_keys.push_back(ColumnSlice(ygg::to_uint_t(position)));
        }
    }
    return plan;
}

urd::data<SetPlan> make_set_plan(std::span<const urd::data<ygg::ColumnLayout>> left, std::span<const urd::data<ygg::ColumnLayout>> right)
{
    ygg::validate_columns(left);
    if (!std::ranges::equal(left, right))
        throw std::invalid_argument("Relational operation: input schemas must have the same column order.");
    auto plan = urd::data<SetPlan> {};
    plan.output_columns.set(left.begin(), left.end());
    return plan;
}

urd::data<SelectEqualPlan>
make_select_equal_plan(std::span<const urd::data<ygg::ColumnLayout>> input, urd::index<ygg::Column> left, urd::index<ygg::Column> right)
{
    auto plan = urd::data<SelectEqualPlan> {};
    plan.left_position = column_position(input, left);
    plan.right_position = column_position(input, right);
    ygg::validate_columns(input);
    if (input[plan.left_position].type != input[plan.right_position].type)
        throw std::invalid_argument("Relational operation: selected columns must have the same type.");
    plan.output_columns.set(input.begin(), input.end());
    return plan;
}

urd::data<SelectValuePlan>
make_select_value_plan(std::span<const urd::data<ygg::ColumnLayout>> input, urd::index<ygg::Column> column, ygg::TypeId type, ygg::Value value)
{
    auto plan = urd::data<SelectValuePlan> {};
    plan.position = column_position(input, column);
    plan.value = value;
    ygg::validate_columns(input);
    if (input[plan.position].type != type)
        throw std::invalid_argument("Relational operation: selected value has an incompatible column type.");
    plan.output_columns.set(input.begin(), input.end());
    return plan;
}
}  // namespace runir::kr::database
