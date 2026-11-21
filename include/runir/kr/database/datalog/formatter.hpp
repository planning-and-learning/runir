/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_DATALOG_FORMATTER_HPP_
#define RUNIR_KR_DATABASE_DATALOG_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/database/datalog/declarations.hpp"

#include <fmt/format.h>

#if YGG_ENABLE_FMT_FORMATTERS
namespace fmt
{
/// COUNT, SUM, MIN or MAX.
template<>
struct formatter<runir::kr::database::datalog::AggregateFunction, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(runir::kr::database::datalog::AggregateFunction function, fmt::format_context& ctx) const;
};

/// A variable as ?n, a constant as its value.
template<>
struct formatter<runir::kr::database::datalog::Term, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::database::datalog::Term& term, fmt::format_context& ctx) const;
};

/// lhs op rhs.
template<>
struct formatter<runir::kr::database::datalog::Comparison, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::database::datalog::Comparison& comparison, fmt::format_context& ctx) const;
};

/// ?result = lhs op rhs.
template<>
struct formatter<runir::kr::database::datalog::Arithmetic, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::database::datalog::Arithmetic& arithmetic, fmt::format_context& ctx) const;
};
}  // namespace fmt
#endif

#endif
