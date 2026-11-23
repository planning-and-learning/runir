/*
 * Copyright (C) 2026 Dominik Drexler
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef RUNIR_KR_DATABASE_SEMANTICS_GENERIC_JOIN_HPP_
#define RUNIR_KR_DATABASE_SEMANTICS_GENERIC_JOIN_HPP_

#include "runir/kr/database/semantics/operations.hpp"
#include "runir/kr/database/semantics/plan_construction.hpp"

#include <cassert>
#include <cista/containers/vector.h>
#include <memory>
#include <span>
#include <tuple>
#include <urd/elements/clone.hpp>
#include <urd/memory.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <vector>
#include <yggdrasil/formalism/free_relations.hpp>
#include <yggdrasil/formalism/values.hpp>

namespace runir::kr::database::detail
{
// Test targets enable these counters consistently across all translation units.
struct GenericJoinWork
{
    size_t candidate_keys = 0;
    size_t probes = 0;
    size_t prefixes = 0;
};

inline GenericJoinWork& generic_join_work() noexcept
{
    thread_local GenericJoinWork work;
    return work;
}

void count_generic_join_work(size_t GenericJoinWork::* member) noexcept;

struct GenericJoinTrie;
}  // namespace runir::kr::database::detail

/// A trie node: what its memory_usage() counts (its table and children).
template<>
struct urd::memory_of<runir::kr::database::detail::GenericJoinTrie>
{
    static std::size_t bytes(const runir::kr::database::detail::GenericJoinTrie& trie);
};

namespace runir::kr::database::detail
{

/// Hash-trie nodes own distinct canonical values; input row movement is irrelevant.
struct GenericJoinTrie
{
    urd::Store<urd::map<ygg::Value, std::unique_ptr<GenericJoinTrie>>> children;
    size_t rows = 0;

    [[nodiscard]] const GenericJoinTrie* find(ygg::Value value) const
    {
        const auto found = children.find(value);
        return found == children.end() ? nullptr : found->second.get();
    }

    /// Whether the trie holds the row's key values (its values at keys, in order).
    template<ygg::RowValues R>
    [[nodiscard]] bool contains(const R& row, std::span<const ColumnSlice> keys) const
    {
        const auto* node = this;
        for (const auto key : keys)
        {
            node = node->find(field_value(row, key.value));
            if (!node)
                return false;
        }
        return node->rows != 0;
    }

    /// Adds the row's key values; whether they were new.
    template<ygg::RowValues R>
    bool insert(const R& row, std::span<const ColumnSlice> keys)
    {
        if (keys.empty())
        {
            const bool inserted = rows == 0;
            rows = 1;
            return inserted;
        }
        const auto value = field_value(row, keys.front().value);
        auto found = children.find(value);
        if (found == children.end())
            found = children.emplace(value, std::make_unique<GenericJoinTrie>()).first;
        if (!found->second->insert(row, keys.subspan(1)))
            return false;
        ++rows;
        return true;
    }

    /// Removes the row's key values; whether the trie held them.
    template<ygg::RowValues R>
    bool erase(const R& row, std::span<const ColumnSlice> keys)
    {
        if (keys.empty())
        {
            const bool erased = rows != 0;
            rows = 0;
            return erased;
        }
        const auto found = children.find(field_value(row, keys.front().value));
        if (found == children.end() || !found->second->erase(row, keys.subspan(1)))
            return false;
        --rows;
        if (found->second->rows == 0)
            children.erase(found);
        return true;
    }

    void clear()
    {
        children.clear();
        rows = 0;
    }

    /// Its table, and each key's bytes and child node.
    [[nodiscard]] size_t memory_usage() const { return urd::memory_usage(children); }
};
}  // namespace runir::kr::database::detail

inline std::size_t urd::memory_of<runir::kr::database::detail::GenericJoinTrie>::bytes(const runir::kr::database::detail::GenericJoinTrie& trie)
{ return trie.memory_usage(); }

namespace runir::kr::database
{

namespace incremental
{
class GenericJoinEvaluator;
}

/// Sequential scratch and hash tries for one prepared plan. Owns indexed values;
/// sources need only remain valid during the call. Moving preserves usability.
class GenericJoinWorkspace
{
    /// A clone of its plan: a workspace is cached by the plans it matches (matches()), so it can outlive the plan it was
    /// made for.
    urd::data<GenericJoinPlan> m_plan;
    urd::Store<urd::sequence<detail::GenericJoinTrie>> m_tries;
    urd::Store<urd::sequence<const detail::GenericJoinTrie*>> m_cursors;
    std::vector<ygg::Value> m_row;  ///< An output row's values, set column by column.

    void enumerate(size_t depth, ygg::FreeScratchRelation& out);

    /// Appends the full assignments over the tries returned by root(input).
    template<std::invocable<size_t> Root>
    void append(ygg::FreeScratchRelation& out, Root root)
    {
        for (size_t input = 0; input < m_tries.size(); ++input)
        {
            const detail::GenericJoinTrie& trie = root(input);
            if (trie.rows == 0)
                return;
            m_cursors[input] = &trie;
        }
        m_row.assign(out.view().arity(), ygg::Value {});
        enumerate(0, out);
    }

    void append(ygg::FreeScratchRelation& out)
    {
        append(out, [&](size_t input) -> const auto& { return m_tries[input]; });
    }

    /// Appends the assignments that read the replacement trie for the replaced input.
    void append(ygg::FreeScratchRelation& out, size_t replaced, const detail::GenericJoinTrie& replacement)
    {
        append(out, [&](size_t input) -> const auto& { return input == replaced ? replacement : m_tries[input]; });
    }

    detail::GenericJoinTrie& trie(size_t input) { return m_tries.at(input); }

    friend class incremental::GenericJoinEvaluator;

public:
    explicit GenericJoinWorkspace(const urd::data<GenericJoinPlan>& plan) :
        m_plan(urd::clone(plan)),
        m_tries(plan.input_columns.size()),
        m_cursors((static_cast<std::size_t>(plan.variable_order.size()) + 1) * plan.input_columns.size())
    {
    }

    [[nodiscard]] bool matches(const urd::data<GenericJoinPlan>& plan) const { return runir::kr::database::matches(m_plan, plan); }

    /// Indexes the inputs, which match the plan, and appends their join to out.
    template<RelationViewRange R>
    void append_join(const R& inputs, ygg::FreeScratchRelation& out)
    {
        for (size_t input = 0; input < m_plan.input_columns.size(); ++input)
        {
            auto& trie = m_tries[input];
            trie.clear();
            for (const auto& held : inputs[input].get_rows())
                ygg::with_values(held, [&](const auto& values) { trie.insert(values, m_plan.input_keys[input]); });
        }
        append(out);
    }

    [[nodiscard]] size_t memory_usage() const { return urd::memory_usage(m_tries) + urd::memory_usage(m_cursors); }
};

/// Generic Join (Ngo, Re, Rudra, 2013, Section 4): intersect one variable at a
/// time, iterating the smallest distinct prefix extension and probing the rest.
/// https://arxiv.org/abs/1310.3314. Hash costs are expected; no ordering is required.
/// Output replaces its rows; an output aliasing an input throws before mutation. The inputs, the output and the
/// workspace are the plan's (asserted in debug builds).
template<RelationViewRange R>
void generic_join(const R& inputs, const urd::data<GenericJoinPlan>& plan, ygg::FreeScratchRelation& out, GenericJoinWorkspace& workspace)
{
    assert(std::ranges::size(inputs) == plan.input_columns.size() && workspace.matches(plan));
    assert(std::ranges::equal(out.view().columns(), plan.output_columns));
    for (size_t input = 0; input < plan.input_columns.size(); ++input)
    {
        assert(std::ranges::equal(inputs[input].columns(), plan.input_columns[input]));
        if (reads(inputs[input], out))
            throw std::invalid_argument("Generic join: output aliases an input.");
    }
    out.clear();
    workspace.append_join(inputs, out);
}
}  // namespace runir::kr::database

#endif
