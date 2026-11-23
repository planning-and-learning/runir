/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "runir/kr/database/syntax/formatter.hpp"

namespace runir::kr::database
{
namespace
{
template<typename OutputIt>
OutputIt format_query_plan(OutputIt out, const QueryPlan& plan)
{
    // Plain values, so explanations do not depend on the optional public formatters.
    const auto value = [](auto index) { return index.value; };
    for (size_t id = 0; id < plan.node_count(); ++id)
        std::visit(
            [&]<typename Tag>(QueryView<Tag> operation)
            {
                urd::Store<urd::sequence<ygg::uint_t>> children;
                for_each_child(operation, [&](QueryView<> child) { children.push_back(child.index().value); });
                const auto columns =
                    operation.columns()
                    | std::views::transform([](const urd::data<ygg::ColumnLayout>& column) { return fmt::format("{}:{}", column.label.value, column.type); });
                out = fmt::format_to(out, "{}: {}({}) columns=[{}]", id, Tag::operation, fmt::join(children, ","), fmt::join(columns, ","));
                if constexpr (std::same_as<Tag, QueryInputTag>)
                    out = fmt::format_to(out, " slot={}", operation.get_input_slot());
                if constexpr (std::same_as<Tag, QueryGenericJoinTag>)
                    out = fmt::format_to(out, " variables=[{}]", fmt::join(operation.get_variable_order() | std::views::transform(value), ","));
                out = fmt::format_to(out, "\n");
            },
            plan[urd::index<Query<>>(ygg::to_uint_t(id))].get_variant());
    return fmt::format_to(out, "roots=[{}]\n", fmt::join(plan.roots() | std::views::transform(value), ","));
}
}  // namespace

std::string explain(const QueryPlan& plan)
{
    fmt::memory_buffer out;
    format_query_plan(std::back_inserter(out), plan);
    return fmt::to_string(out);
}
}

#if YGG_ENABLE_FMT_FORMATTERS
fmt::format_context::iterator fmt::formatter<runir::kr::database::QueryPlan>::format(const runir::kr::database::QueryPlan& plan, fmt::format_context& ctx) const
{ return runir::kr::database::format_query_plan(ctx.out(), plan); }

fmt::format_context::iterator fmt::formatter<runir::kr::database::ColumnSlice>::format(runir::kr::database::ColumnSlice slice, fmt::format_context& ctx) const
{ return fmt::format_to(ctx.out(), "{}", slice.value); }
#endif
