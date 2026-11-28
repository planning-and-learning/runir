/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_DATALOG_VIEW_HPP_
#define RUNIR_KR_DATABASE_DATALOG_VIEW_HPP_

#include "runir/kr/database/datalog/data.hpp"

#include <span>
#include <string_view>
#include <urd/elements/view.hpp>

namespace urd
{
template<typename V>
struct view_mixin<runir::kr::database::datalog::Relation, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] size_t id() const { return self().template member<&urd::record_of<V>::id>(); }

    [[nodiscard]] size_t arity() const { return self().template member<&urd::record_of<V>::arity>(); }

    [[nodiscard]] bool input() const { return self().template member<&urd::record_of<V>::input>(); }

    [[nodiscard]] size_t input_slot() const { return self().template member<&urd::record_of<V>::input_slot>(); }

    [[nodiscard]] std::string_view name() const noexcept { return std::string_view(self().template member<&urd::record_of<V>::name>()); }

    [[nodiscard]] bool weighted() const { return self().template member<&urd::record_of<V>::weighted>(); }
};

template<typename V>
struct view_mixin<runir::kr::database::datalog::Atom, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] auto relation() const { return self().template member<&urd::record_of<V>::relation>(); }

    [[nodiscard]] std::span<const runir::kr::database::datalog::Term> terms() const
    { return { self().template member<&urd::record_of<V>::terms>().data(), self().template member<&urd::record_of<V>::terms>().size() }; }
};

template<typename V>
struct view_mixin<runir::kr::database::datalog::Aggregate, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] auto function() const { return self().template member<&urd::record_of<V>::function>(); }

    [[nodiscard]] ygg::uint_t result() const { return self().template member<&urd::record_of<V>::result>(); }

    [[nodiscard]] ygg::uint_t value() const { return self().template member<&urd::record_of<V>::value>(); }

    [[nodiscard]] auto atoms() const { return self().template member<&urd::record_of<V>::atoms>(); }

    [[nodiscard]] auto negated() const { return self().template member<&urd::record_of<V>::negated>(); }

    [[nodiscard]] std::span<const runir::kr::database::datalog::Comparison> comparisons() const
    { return { self().template member<&urd::record_of<V>::comparisons>().data(), self().template member<&urd::record_of<V>::comparisons>().size() }; }

    [[nodiscard]] std::span<const runir::kr::database::datalog::Arithmetic> arithmetic() const
    { return { self().template member<&urd::record_of<V>::arithmetic>().data(), self().template member<&urd::record_of<V>::arithmetic>().size() }; }
};

template<typename V>
struct view_mixin<runir::kr::database::datalog::Rule, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] auto head() const { return self().template member<&urd::record_of<V>::head>(); }

    [[nodiscard]] auto atoms() const { return self().template member<&urd::record_of<V>::atoms>(); }

    [[nodiscard]] auto negated() const { return self().template member<&urd::record_of<V>::negated>(); }

    [[nodiscard]] std::span<const runir::kr::database::datalog::Comparison> comparisons() const
    { return { self().template member<&urd::record_of<V>::comparisons>().data(), self().template member<&urd::record_of<V>::comparisons>().size() }; }

    [[nodiscard]] auto aggregates() const { return self().template member<&urd::record_of<V>::aggregates>(); }

    [[nodiscard]] std::span<const runir::kr::database::datalog::Arithmetic> arithmetic() const
    { return { self().template member<&urd::record_of<V>::arithmetic>().data(), self().template member<&urd::record_of<V>::arithmetic>().size() }; }
};

template<typename V>
struct view_mixin<runir::kr::database::datalog::Program, V>
{
private:
    [[nodiscard]] const V& self() const noexcept { return static_cast<const V&>(*this); }

public:
    [[nodiscard]] auto rules() const { return self().template member<&urd::record_of<V>::rules>(); }
};
}  // namespace urd

namespace runir::kr::database::datalog
{
using RelationView = urd::index_view<Relation, DatalogRepository>;
using AtomView = urd::index_view<Atom, DatalogRepository>;
using AggregateView = urd::index_view<Aggregate, DatalogRepository>;
using RuleView = urd::index_view<Rule, DatalogRepository>;
using ProgramView = urd::index_view<Program, DatalogRepository>;
}  // namespace runir::kr::database::datalog

#endif
