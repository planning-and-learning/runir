/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef RUNIR_KR_DATABASE_SYNTAX_FORMATTER_HPP_
#define RUNIR_KR_DATABASE_SYNTAX_FORMATTER_HPP_

#include "runir/formatter.hpp"
#include "runir/kr/database/syntax/query.hpp"
#include "yggdrasil/formalism/formatter.hpp"
#include "yggdrasil/formatting/formatter.hpp"

#include <boost/mp11.hpp>
#include <fmt/ranges.h>
#include <iterator>
#include <ranges>
#include <string>
#include <urd/format.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>

namespace runir::kr::database
{
/// The plan as text: each node by its number, its operation, children and columns (an input its slot, a generic join its
/// variable order), then its roots.
std::string explain(const QueryPlan& plan);
}  // namespace runir::kr::database

#if YGG_ENABLE_FMT_FORMATTERS
namespace fmt
{
/// A query plan, as explain writes it.
template<>
struct formatter<runir::kr::database::QueryPlan, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const runir::kr::database::QueryPlan& plan, fmt::format_context& ctx) const;
};

/// An operation's plan, as the columns it produces: how a query's record writes it.
template<typename Plan>
    requires boost::mp11::mp_contains<boost::mp11::mp_list<runir::kr::database::ProjectionPlan,
                                                           runir::kr::database::JoinPlan,
                                                           runir::kr::database::SetPlan,
                                                           runir::kr::database::SelectEqualPlan,
                                                           runir::kr::database::SelectValuePlan,
                                                           runir::kr::database::DistancePlan,
                                                           runir::kr::database::GenericJoinPlan>,
                                      Plan>::value
struct formatter<Plan, char> : runir::detail::without_format_spec
{
    fmt::format_context::iterator format(const Plan& plan, fmt::format_context& ctx) const;
};
}  // namespace fmt
#endif

#endif
