/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef RUNIR_KR_DATABASE_SEMANTICS_PLAN_CONSTRUCTION_HPP_
#define RUNIR_KR_DATABASE_SEMANTICS_PLAN_CONSTRUCTION_HPP_

#include "runir/kr/database/semantics/distance_plan_data.hpp"
#include "runir/kr/database/semantics/generic_join_plan_data.hpp"
#include "runir/kr/database/semantics/plan_data.hpp"

#include <cstddef>
#include <span>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

/// The plans made from their inputs' columns, which they validate (make_*), and what is computed from a plan.
namespace runir::kr::database
{
/// Keeps the input's columns `columns` in that order (each an input label, once).
urd::data<ProjectionPlan> make_projection_plan(std::span<const urd::data<ygg::ColumnLayout>> input, std::span<const urd::index<ygg::Column>> columns);

/// Joins inputs of the columns `left` and `right` on their common labels.
urd::data<JoinPlan> make_join_plan(std::span<const urd::data<ygg::ColumnLayout>> left, std::span<const urd::data<ygg::ColumnLayout>> right);

/// Unites or subtracts inputs of the columns `left` and `right`, which are the same.
urd::data<SetPlan> make_set_plan(std::span<const urd::data<ygg::ColumnLayout>> left, std::span<const urd::data<ygg::ColumnLayout>> right);

/// Selects the rows of `input` whose columns `left` and `right` (of the same type) hold equal values.
urd::data<SelectEqualPlan>
make_select_equal_plan(std::span<const urd::data<ygg::ColumnLayout>> input, urd::index<ygg::Column> left, urd::index<ygg::Column> right);

/// Selects the rows of `input` whose column `column` holds `value` (canonical, encode_value) of type `type`, the column's.
urd::data<SelectValuePlan>
make_select_value_plan(std::span<const urd::data<ygg::ColumnLayout>> input, urd::index<ygg::Column> column, ygg::TypeId type, ygg::Value value);

/// Joins inputs of the columns `inputs`, enumerating their variables in `variable_order`, its output in `output_order`.
urd::data<GenericJoinPlan> make_generic_join_plan(std::span<const urd::Store<urd::sequence<urd::data<ygg::ColumnLayout>>>> inputs,
                                                  std::span<const urd::index<ygg::Column>> variable_order,
                                                  std::span<const urd::index<ygg::Column>> output_order);

/// Whether `plan` and `other` join inputs of the same columns in the same variable and output orders.
[[nodiscard]] bool matches(const urd::data<GenericJoinPlan>& plan, const urd::data<GenericJoinPlan>& other);

/// Distances from `sources` to `targets` over `edges`, written to the column `distance_column`.
urd::data<DistancePlan> make_distance_plan(std::span<const urd::data<ygg::ColumnLayout>> sources,
                                           std::span<const urd::data<ygg::ColumnLayout>> edges,
                                           std::span<const urd::data<ygg::ColumnLayout>> targets,
                                           urd::index<ygg::Column> distance_column);

/// The number of fields of a vertex, k.
[[nodiscard]] inline size_t tuple_size(const urd::data<DistancePlan>& plan) noexcept { return plan.source_columns.size(); }

/// The number of fields of a vertex, k.
[[nodiscard]] inline size_t arity(const urd::data<DistancePlan>& plan) noexcept { return plan.source_columns.size(); }
}  // namespace runir::kr::database

#endif
