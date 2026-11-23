/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef RUNIR_KR_DATABASE_SEMANTICS_GENERIC_JOIN_PLAN_DATA_HPP_
#define RUNIR_KR_DATABASE_SEMANTICS_GENERIC_JOIN_PLAN_DATA_HPP_

#include "runir/kr/database/semantics/plan_data.hpp"

#include <cstddef>
#include <span>
#include <urd/elements/data.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>

/// Prepared full natural join. Variable order controls execution; output order
/// independently preserves the caller's schema. Projection is a separate operator.
template<>
struct urd::data<runir::kr::database::GenericJoinPlan>
{
    /// The columns of each input.
    urd::flat_vector<urd::flat_vector<urd::data<ygg::ColumnLayout>>> input_columns;
    urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns;
    urd::flat_vector<urd::index<ygg::Column>> variable_order;
    /// Of each input, the positions of its columns in variable order.
    urd::flat_vector<urd::flat_vector<runir::kr::database::ColumnSlice>> input_keys;
    /// Of each variable (by its depth), the inputs that have its column.
    urd::flat_vector<urd::flat_vector<size_t>> variable_inputs;
    /// Of each variable (by its depth), its column's position in the output.
    urd::flat_vector<runir::kr::database::ColumnSlice> output_positions;

    data() = default;

    data(urd::flat_vector<urd::flat_vector<urd::data<ygg::ColumnLayout>>> input_columns_,
         urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns_,
         urd::flat_vector<urd::index<ygg::Column>> variable_order_,
         urd::flat_vector<urd::flat_vector<runir::kr::database::ColumnSlice>> input_keys_,
         urd::flat_vector<urd::flat_vector<size_t>> variable_inputs_,
         urd::flat_vector<runir::kr::database::ColumnSlice> output_positions_) :
        input_columns(std::move(input_columns_)),
        output_columns(std::move(output_columns_)),
        variable_order(std::move(variable_order_)),
        input_keys(std::move(input_keys_)),
        variable_inputs(std::move(variable_inputs_)),
        output_positions(std::move(output_positions_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"input_columns", &data::input_columns>,
                           urd::field<"output_columns", &data::output_columns>,
                           urd::field<"variable_order", &data::variable_order>,
                           urd::field<"input_keys", &data::input_keys>,
                           urd::field<"variable_inputs", &data::variable_inputs>,
                           urd::field<"output_positions", &data::output_positions>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept
    {
        return urd::
            Members<&data::input_columns, &data::output_columns, &data::variable_order, &data::input_keys, &data::variable_inputs, &data::output_positions> {};
    }
};

#endif
