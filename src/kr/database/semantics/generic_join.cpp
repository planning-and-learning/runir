/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "runir/kr/database/semantics/generic_join.hpp"

#include <algorithm>
#include <span>
#include <stdexcept>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>

namespace runir::kr::database
{
urd::data<GenericJoinPlan> make_generic_join_plan(std::span<const urd::Store<urd::sequence<urd::data<ygg::ColumnLayout>>>> inputs,
                                                  std::span<const urd::index<ygg::Column>> variable_order,
                                                  std::span<const urd::index<ygg::Column>> output_order)
{
    auto plan = urd::data<GenericJoinPlan> {};
    plan.input_columns.resize(inputs.size());
    plan.variable_order.set(variable_order.begin(), variable_order.end());
    plan.input_keys.resize(inputs.size());
    plan.variable_inputs.resize(variable_order.size());
    for (size_t i = 0; i < inputs.size(); ++i)
        plan.input_columns[i].set(inputs[i].begin(), inputs[i].end());
    urd::Store<urd::sequence<urd::data<ygg::ColumnLayout>>> combined;
    for (const auto& input : inputs)
    {
        ygg::validate_columns(input);
        for (const auto column : input)
        {
            const auto found = std::ranges::find(combined, column.label, &urd::data<ygg::ColumnLayout>::label);
            if (found == combined.end())
                combined.push_back(column);
            else if (found->type != column.type)
                throw std::invalid_argument("Generic join: common columns must have the same type.");
        }
    }
    const auto require_order = [&](std::span<const urd::index<ygg::Column>> order)
    {
        if (order.size() != combined.size())
            throw std::invalid_argument("Generic join: orders must contain every output column exactly once.");
        for (size_t i = 0; i < order.size(); ++i)
            if (std::ranges::find(combined, order[i], &urd::data<ygg::ColumnLayout>::label) == combined.end()
                || std::ranges::find(order.first(i), order[i]) != order.first(i).end())
                throw std::invalid_argument("Generic join: orders must contain every output column exactly once.");
    };
    require_order(variable_order);
    require_order(output_order);
    urd::Store<urd::sequence<urd::data<ygg::ColumnLayout>>> output;
    for (const auto label : output_order)
        output.push_back(*std::ranges::find(combined, label, &urd::data<ygg::ColumnLayout>::label));
    plan.output_columns.set(output.begin(), output.end());
    for (size_t depth = 0; depth < variable_order.size(); ++depth)
    {
        const auto label = variable_order[depth];
        plan.output_positions.push_back(
            ColumnSlice(ygg::to_uint_t(static_cast<size_t>(std::ranges::find(output, label, &urd::data<ygg::ColumnLayout>::label) - output.begin()))));
        for (size_t input = 0; input < inputs.size(); ++input)
        {
            const auto found = std::ranges::find(inputs[input], label, &urd::data<ygg::ColumnLayout>::label);
            if (found != inputs[input].end())
            {
                plan.input_keys[input].push_back(ColumnSlice(ygg::to_uint_t(static_cast<size_t>(found - inputs[input].begin()))));
                plan.variable_inputs[depth].push_back(input);
            }
        }
    }
    return plan;
}

bool matches(const urd::data<GenericJoinPlan>& plan, const urd::data<GenericJoinPlan>& other)
{ return plan.input_columns == other.input_columns && plan.variable_order == other.variable_order && plan.output_columns == other.output_columns; }

namespace detail
{
void count_generic_join_work(size_t GenericJoinWork::* member) noexcept { ++(generic_join_work().*member); }
}
}

namespace runir::kr::database
{
void GenericJoinWorkspace::enumerate(size_t depth, ygg::FreeScratchRelation& out)
{
    detail::count_generic_join_work(&detail::GenericJoinWork::prefixes);
    if (depth == m_plan.variable_order.size())
    {
        out.insert(std::span<const ygg::Value>(m_row));
        return;
    }
    const auto count = m_plan.input_columns.size();
    const auto cursors = std::span(m_cursors);
    const auto current = cursors.subspan(depth * count, count);
    const auto next = cursors.subspan((depth + 1) * count, count);
    const auto& inputs = m_plan.variable_inputs[depth];
    const auto source = *std::ranges::min_element(inputs, {}, [&](size_t input) { return current[input]->children.size(); });
    for (const auto& [key, child] : current[source]->children)
    {
        detail::count_generic_join_work(&detail::GenericJoinWork::candidate_keys);
        std::ranges::copy(current, next.begin());
        bool present = true;
        for (const auto input : inputs)
        {
            if (input != source)
                detail::count_generic_join_work(&detail::GenericJoinWork::probes);
            next[input] = input == source ? child.get() : current[input]->find(key);
            if (!next[input])
            {
                present = false;
                break;
            }
        }
        if (present)
        {
            m_row[m_plan.output_positions[depth].value] = key;
            enumerate(depth + 1, out);
        }
    }
}
}
