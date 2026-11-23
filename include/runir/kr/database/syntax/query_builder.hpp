/*
 * Copyright (C) 2025-2026 Dominik Drexler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef RUNIR_KR_DATABASE_SYNTAX_QUERY_BUILDER_HPP_
#define RUNIR_KR_DATABASE_SYNTAX_QUERY_BUILDER_HPP_

#include "runir/kr/database/syntax/query_data.hpp"
#include "runir/kr/database/syntax/query_view.hpp"

#include <optional>
#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>
#include <variant>
#include <vector>

namespace urd
{

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::Query<>, C>
{
    urd::key<runir::kr::database::Query<>> index;
    urd::built_t<urd::index_variant<boost::mp11::mp_transform<runir::kr::database::Query, runir::kr::database::QueryConstructorTags>>, C> variant;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"variant", &builder::variant>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::variant> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::Query<runir::kr::database::QueryInputTag>, C>
{
    urd::key<runir::kr::database::Query<runir::kr::database::QueryInputTag>> index;
    urd::built_t<size_t, C> input_slot = 0;
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> columns;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"input_slot", &builder::input_slot>, urd::field<"columns", &builder::columns>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::input_slot, &builder::columns> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::Query<runir::kr::database::QueryEmptyTag>, C>
{
    urd::key<runir::kr::database::Query<runir::kr::database::QueryEmptyTag>> index;
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> columns;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"columns", &builder::columns>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::columns> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename Tag, typename C>
    requires boost::mp11::mp_contains<runir::kr::database::QueryJoinTags, Tag>::value
struct builder<runir::kr::database::Query<Tag>, C>
{
    urd::key<runir::kr::database::Query<Tag>> index;
    urd::built_t<urd::index<runir::kr::database::Query<>>, C> left_operand;
    urd::built_t<urd::index<runir::kr::database::Query<>>, C> right_operand;
    urd::built_t<runir::kr::database::JoinPlan, C> plan;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_operand", &builder::left_operand>,
                           urd::field<"right_operand", &builder::right_operand>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::left_operand, &builder::right_operand> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename Tag, typename C>
    requires boost::mp11::mp_contains<runir::kr::database::QuerySetTags, Tag>::value
struct builder<runir::kr::database::Query<Tag>, C>
{
    urd::key<runir::kr::database::Query<Tag>> index;
    urd::built_t<urd::index<runir::kr::database::Query<>>, C> left_operand;
    urd::built_t<urd::index<runir::kr::database::Query<>>, C> right_operand;
    urd::built_t<runir::kr::database::SetPlan, C> plan;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_operand", &builder::left_operand>,
                           urd::field<"right_operand", &builder::right_operand>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::left_operand, &builder::right_operand> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::Query<runir::kr::database::QueryProjectTag>, C>
{
    urd::key<runir::kr::database::Query<runir::kr::database::QueryProjectTag>> index;
    urd::built_t<urd::index<runir::kr::database::Query<>>, C> operand;
    urd::built_t<urd::flat_vector<urd::index<ygg::Column>>, C> labels;
    urd::built_t<runir::kr::database::ProjectionPlan, C> plan;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &builder::operand>, urd::field<"labels", &builder::labels>, urd::field<"plan", &builder::plan>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::labels> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::Query<runir::kr::database::QueryRenameTag>, C>
{
    urd::key<runir::kr::database::Query<runir::kr::database::QueryRenameTag>> index;
    urd::built_t<urd::index<runir::kr::database::Query<>>, C> operand;
    urd::built_t<urd::flat_vector<urd::index<ygg::Column>>, C> labels;
    urd::built_t<urd::flat_vector<urd::data<ygg::ColumnLayout>>, C> columns;

    static constexpr auto fields() noexcept
    { return urd::Fields<urd::field<"operand", &builder::operand>, urd::field<"labels", &builder::labels>, urd::field<"columns", &builder::columns>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::labels> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::Query<runir::kr::database::QuerySelectEqualTag>, C>
{
    urd::key<runir::kr::database::Query<runir::kr::database::QuerySelectEqualTag>> index;
    urd::built_t<urd::index<runir::kr::database::Query<>>, C> operand;
    urd::built_t<urd::index<ygg::Column>, C> left_column;
    urd::built_t<urd::index<ygg::Column>, C> right_column;
    urd::built_t<runir::kr::database::SelectEqualPlan, C> plan;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"operand", &builder::operand>,
                           urd::field<"left_column", &builder::left_column>,
                           urd::field<"right_column", &builder::right_column>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::operand, &builder::left_column, &builder::right_column> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::Query<runir::kr::database::QuerySelectValueTag>, C>
{
    urd::key<runir::kr::database::Query<runir::kr::database::QuerySelectValueTag>> index;
    urd::built_t<urd::index<runir::kr::database::Query<>>, C> operand;
    urd::built_t<urd::index<ygg::Column>, C> column;
    urd::built_t<ygg::Value, C> constant = 0;
    urd::built_t<ygg::TypeId, C> constant_type {};
    urd::built_t<runir::kr::database::SelectValuePlan, C> plan;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"operand", &builder::operand>,
                           urd::field<"column", &builder::column>,
                           urd::field<"constant", &builder::constant>,
                           urd::field<"constant_type", &builder::constant_type>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept
    { return urd::Members<&builder::operand, &builder::column, &builder::constant, &builder::constant_type> {}; }
};

/// The builder of Query: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::Query<runir::kr::database::QueryGenericJoinTag>, C>
{
    urd::key<runir::kr::database::Query<runir::kr::database::QueryGenericJoinTag>> index;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::database::Query<>>>, C> inputs;
    urd::built_t<urd::flat_vector<urd::index<ygg::Column>>, C> variable_order;
    urd::built_t<urd::flat_vector<urd::index<ygg::Column>>, C> output_order;
    urd::built_t<runir::kr::database::GenericJoinPlan, C> plan;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"inputs", &builder::inputs>,
                           urd::field<"variable_order", &builder::variable_order>,
                           urd::field<"output_order", &builder::output_order>,
                           urd::field<"plan", &builder::plan>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::Members<&builder::inputs, &builder::variable_order, &builder::output_order> {}; }
};

}  // namespace urd

#endif
