/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef RUNIR_KR_DATABASE_SEMANTICS_PLAN_DATA_HPP_
#define RUNIR_KR_DATABASE_SEMANTICS_PLAN_DATA_HPP_

#include "runir/kr/database/declarations.hpp"
#include "yggdrasil/core/unsigned_integer.hpp"
#include "yggdrasil/formalism/relation_view.hpp"

#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <tuple>
#include <urd/elements/data.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>

namespace runir::kr::database
{

/// A field: its column's position in the row, independent of its column label. Renaming columns preserves compiled keys
/// and reusable join indexes.
struct ColumnSlice : ygg::UnsignedInteger<ColumnSlice>
{
    using UnsignedInteger::UnsignedInteger;
};

}  // namespace runir::kr::database

/// A prepared operation is built from its inputs' columns, which it validates (and owns), and resolves positions once.
/// Reuse it with any inputs of the same ordered columns, whatever their rows or identity; the operations only assert
/// (debug builds) that their inputs' and output's columns are the plan's.

/// Keeps the input's columns in the order of `positions` (each an input column, once).
template<>
struct urd::data<runir::kr::database::ProjectionPlan>
{
    urd::flat_vector<urd::data<ygg::ColumnLayout>> input_columns;
    urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns;
    urd::flat_vector<runir::kr::database::ColumnSlice> positions;

    data() = default;

    data(urd::flat_vector<urd::data<ygg::ColumnLayout>> input_columns_,
         urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns_,
         urd::flat_vector<runir::kr::database::ColumnSlice> positions_) :
        input_columns(std::move(input_columns_)),
        output_columns(std::move(output_columns_)),
        positions(std::move(positions_))
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
                           urd::field<"positions", &data::positions>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::input_columns, &data::output_columns, &data::positions> {}; }
};

/// Natural join (and semijoin, antijoin) keys and payload: common labels must have the same type. The output is the left
/// columns followed by the right-only ones. The smaller hash-build side is selected from current input sizes at runtime.
template<>
struct urd::data<runir::kr::database::JoinPlan>
{
    urd::flat_vector<urd::data<ygg::ColumnLayout>> left_columns;
    urd::flat_vector<urd::data<ygg::ColumnLayout>> right_columns;
    urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns;
    urd::flat_vector<runir::kr::database::ColumnSlice> left_keys;
    urd::flat_vector<runir::kr::database::ColumnSlice> right_keys;
    urd::flat_vector<runir::kr::database::ColumnSlice> right_payload;

    data() = default;

    data(urd::flat_vector<urd::data<ygg::ColumnLayout>> left_columns_,
         urd::flat_vector<urd::data<ygg::ColumnLayout>> right_columns_,
         urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns_,
         urd::flat_vector<runir::kr::database::ColumnSlice> left_keys_,
         urd::flat_vector<runir::kr::database::ColumnSlice> right_keys_,
         urd::flat_vector<runir::kr::database::ColumnSlice> right_payload_) :
        left_columns(std::move(left_columns_)),
        right_columns(std::move(right_columns_)),
        output_columns(std::move(output_columns_)),
        left_keys(std::move(left_keys_)),
        right_keys(std::move(right_keys_)),
        right_payload(std::move(right_payload_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_columns", &data::left_columns>,
                           urd::field<"right_columns", &data::right_columns>,
                           urd::field<"output_columns", &data::output_columns>,
                           urd::field<"left_keys", &data::left_keys>,
                           urd::field<"right_keys", &data::right_keys>,
                           urd::field<"right_payload", &data::right_payload>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::left_columns, &data::right_columns, &data::output_columns, &data::left_keys, &data::right_keys, &data::right_payload> {}; }
};

/// Union and difference: both inputs have the same columns in the same order (project one first to align them).
template<>
struct urd::data<runir::kr::database::SetPlan>
{
    /// The columns of both inputs and of the output.
    urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns;

    data() = default;

    explicit data(urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns_) : output_columns(std::move(output_columns_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"output_columns", &data::output_columns>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::output_columns> {}; }
};

/// The rows whose columns at `left_position` and `right_position` (of the same type) hold equal values.
template<>
struct urd::data<runir::kr::database::SelectEqualPlan>
{
    /// The columns of the input and of the output.
    urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns;
    size_t left_position = 0;
    size_t right_position = 0;

    data() = default;

    data(urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns_, size_t left_position_, size_t right_position_) :
        output_columns(std::move(output_columns_)),
        left_position(left_position_),
        right_position(right_position_)
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"output_columns", &data::output_columns>,
                           urd::field<"left_position", &data::left_position>,
                           urd::field<"right_position", &data::right_position>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::output_columns, &data::left_position, &data::right_position> {}; }
};

/// The rows whose column at `position` holds `value` (canonical, encode_value) of the column's type.
template<>
struct urd::data<runir::kr::database::SelectValuePlan>
{
    /// The columns of the input and of the output.
    urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns;
    size_t position = 0;
    ygg::Value value = 0;

    data() = default;

    data(urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns_, size_t position_, ygg::Value value_) :
        output_columns(std::move(output_columns_)),
        position(position_),
        value(value_)
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"output_columns", &data::output_columns>, urd::field<"position", &data::position>, urd::field<"value", &data::value>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::output_columns, &data::position, &data::value> {}; }
};

#endif
