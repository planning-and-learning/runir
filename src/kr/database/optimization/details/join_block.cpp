/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "runir/kr/database/optimization/details/join_block.hpp"

#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::database::detail
{
urd::Store<urd::sequence<urd::index<ygg::Column>>> variables(QueryView<> query)
{
    auto labels = urd::Store<urd::sequence<urd::index<ygg::Column>>>();
    for (const auto& column : columns_of(query))
        labels.push_back(column.label);
    return ygg::canonicalized(std::move(labels));
}
}

namespace runir::kr::database::detail
{
QueryView<> semijoin(OperatorBuilder& build, QueryView<> left_operand, QueryView<> right_operand)
{ return build.join(left_operand, build.restrict(right_operand, variables(left_operand))); }
}

namespace runir::kr::database::detail
{
urd::Store<urd::sequence<urd::index<ygg::Column>>> variable_order(std::span<const QueryView<>> atoms)
{
    urd::Store<urd::sequence<urd::index<ygg::Column>>> result;
    for (const auto atom : atoms)
        for (const auto label : variables(atom))
            result.push_back(label);
    ygg::canonicalize(result);
    return result;
}
}
