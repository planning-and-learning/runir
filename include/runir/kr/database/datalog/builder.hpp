/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_DATALOG_BUILDER_HPP_
#define RUNIR_KR_DATABASE_DATALOG_BUILDER_HPP_

#include "runir/kr/database/datalog/data.hpp"
#include "runir/kr/database/datalog/view.hpp"

#include <urd/elements/builder.hpp>
#include <urd/elements/children.hpp>

namespace urd
{
/// The builder of Relation: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::datalog::Relation, C>
{
    urd::key<runir::kr::database::datalog::Relation> index;
    urd::built_t<size_t, C> id = 0;
    urd::built_t<size_t, C> arity = 0;
    urd::built_t<bool, C> input = false;
    urd::built_t<size_t, C> input_slot = 0;
    urd::built_t<urd::flat_string, C> name;
    urd::built_t<bool, C> weighted = false;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"id", &builder::id>,
                           urd::field<"arity", &builder::arity>,
                           urd::field<"input", &builder::input>,
                           urd::field<"input_slot", &builder::input_slot>,
                           urd::field<"name", &builder::name>,
                           urd::field<"weighted", &builder::weighted>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Atom: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::datalog::Atom, C>
{
    urd::key<runir::kr::database::datalog::Atom> index;
    urd::built_t<urd::index<runir::kr::database::datalog::Relation>, C> relation;
    urd::built_t<urd::flat_vector<runir::kr::database::datalog::Term>, C> terms;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"relation", &builder::relation>, urd::field<"terms", &builder::terms>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Aggregate: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::datalog::Aggregate, C>
{
    urd::key<runir::kr::database::datalog::Aggregate> index;
    urd::built_t<runir::kr::database::datalog::AggregateFunction, C> function = runir::kr::database::datalog::AggregateFunction::COUNT;
    urd::built_t<ygg::uint_t, C> result = 0;
    urd::built_t<ygg::uint_t, C> value = 0;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::database::datalog::Atom>>, C> atoms;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::database::datalog::Atom>>, C> negated;
    urd::built_t<urd::flat_vector<runir::kr::database::datalog::Comparison>, C> comparisons;
    urd::built_t<urd::flat_vector<runir::kr::database::datalog::Arithmetic>, C> arithmetic;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"function", &builder::function>,
                           urd::field<"result", &builder::result>,
                           urd::field<"value", &builder::value>,
                           urd::field<"atoms", &builder::atoms>,
                           urd::field<"negated", &builder::negated>,
                           urd::field<"comparisons", &builder::comparisons>,
                           urd::field<"arithmetic", &builder::arithmetic>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Rule: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::datalog::Rule, C>
{
    urd::key<runir::kr::database::datalog::Rule> index;
    urd::built_t<urd::index<runir::kr::database::datalog::Atom>, C> head;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::database::datalog::Atom>>, C> atoms;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::database::datalog::Atom>>, C> negated;
    urd::built_t<urd::flat_vector<runir::kr::database::datalog::Comparison>, C> comparisons;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::database::datalog::Aggregate>>, C> aggregates;
    urd::built_t<urd::flat_vector<runir::kr::database::datalog::Arithmetic>, C> arithmetic;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"head", &builder::head>,
                           urd::field<"atoms", &builder::atoms>,
                           urd::field<"negated", &builder::negated>,
                           urd::field<"comparisons", &builder::comparisons>,
                           urd::field<"aggregates", &builder::aggregates>,
                           urd::field<"arithmetic", &builder::arithmetic>> {};
    }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

/// The builder of Program: its record's members, each in its built form (urd::built_t).
template<typename C>
struct builder<runir::kr::database::datalog::Program, C>
{
    urd::key<runir::kr::database::datalog::Program> index;
    urd::built_t<urd::flat_vector<urd::index<runir::kr::database::datalog::Rule>>, C> rules;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"rules", &builder::rules>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
};

}  // namespace urd

#endif
