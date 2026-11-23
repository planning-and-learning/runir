/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef RUNIR_KR_DATABASE_SEMANTICS_DISTANCE_PLAN_DATA_HPP_
#define RUNIR_KR_DATABASE_SEMANTICS_DISTANCE_PLAN_DATA_HPP_

#include "runir/kr/database/declarations.hpp"
#include "yggdrasil/formalism/relation_view.hpp"

#include <cstddef>
#include <span>
#include <urd/elements/data.hpp>
#include <urd/elements/flat.hpp>
#include <utility>

/// Sources and targets contain k-field vertices; each edge contains two such
/// vertices. Positional field types must agree, independently of column labels.
/// Output columns are the edge columns followed by a fresh uint_t distance column.
template<>
struct urd::data<runir::kr::database::DistancePlan>
{
    urd::flat_vector<urd::data<ygg::ColumnLayout>> source_columns;
    urd::flat_vector<urd::data<ygg::ColumnLayout>> edge_columns;
    urd::flat_vector<urd::data<ygg::ColumnLayout>> target_columns;
    urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns;

    data() = default;

    data(urd::flat_vector<urd::data<ygg::ColumnLayout>> source_columns_,
         urd::flat_vector<urd::data<ygg::ColumnLayout>> edge_columns_,
         urd::flat_vector<urd::data<ygg::ColumnLayout>> target_columns_,
         urd::flat_vector<urd::data<ygg::ColumnLayout>> output_columns_) :
        source_columns(std::move(source_columns_)),
        edge_columns(std::move(edge_columns_)),
        target_columns(std::move(target_columns_)),
        output_columns(std::move(output_columns_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"source_columns", &data::source_columns>,
                           urd::field<"edge_columns", &data::edge_columns>,
                           urd::field<"target_columns", &data::target_columns>,
                           urd::field<"output_columns", &data::output_columns>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept
    { return urd::Members<&data::source_columns, &data::edge_columns, &data::target_columns, &data::output_columns> {}; }
};

#endif
