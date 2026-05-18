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

#ifndef RUNIR_DATASETS_SERIALIZATION_CONTEXT_HPP_
#define RUNIR_DATASETS_SERIALIZATION_CONTEXT_HPP_

#include "runir/datasets/serialization/archives.hpp"
#include "runir/datasets/task_class.hpp"

#include <algorithm>
#include <tyr/common/comparators.hpp>
#include <tyr/common/declarations.hpp>
#include <tyr/formalism/planning/parser.hpp>
#include <tyr/planning/planning.hpp>
#include <utility>
#include <vector>

namespace runir::datasets::serialization
{

template<typename T>
struct SymbolTableBuilder
{
    std::vector<T> symbols;
    tyr::UnorderedMap<T, SymbolIndex> symbol_to_index;

    void insert(T symbol) { symbol_to_index.try_emplace(std::move(symbol), 0); }

    void finalize()
    {
        symbols.clear();
        symbols.reserve(symbol_to_index.size());
        for (const auto& [symbol, _] : symbol_to_index)
            symbols.push_back(symbol);
        std::sort(symbols.begin(), symbols.end(), tyr::Less<T> {});

        for (auto index = SymbolIndex { 0 }; index < symbols.size(); ++index)
            symbol_to_index[symbols[index]] = index;
    }

    auto index(const T& symbol) const -> SymbolIndex { return symbol_to_index.at(symbol); }
    auto release() const -> SymbolTableArchive<T> { return SymbolTableArchive<T> { symbols }; }
};

struct SymbolTablesBuilder
{
    SymbolTableBuilder<AtomArchive<tyr::formalism::StaticTag>> static_atoms;
    SymbolTableBuilder<FDRFactArchive<tyr::formalism::FluentTag>> fluent_facts;
    SymbolTableBuilder<AtomArchive<tyr::formalism::DerivedTag>> derived_atoms;
    SymbolTableBuilder<NumericValueArchive<tyr::formalism::StaticTag>> static_numeric_values;
    SymbolTableBuilder<NumericValueArchive<tyr::formalism::FluentTag>> fluent_numeric_values;
    SymbolTableBuilder<GroundActionArchive> ground_actions;

    void finalize()
    {
        static_atoms.finalize();
        fluent_facts.finalize();
        derived_atoms.finalize();
        static_numeric_values.finalize();
        fluent_numeric_values.finalize();
        ground_actions.finalize();
    }

    auto release() const -> SymbolTablesArchive
    {
        return SymbolTablesArchive { static_atoms.release(),          fluent_facts.release(),          derived_atoms.release(),
                                     static_numeric_values.release(), fluent_numeric_values.release(), ground_actions.release() };
    }
};

struct StateSerializationContext
{
    SymbolTablesBuilder* symbols;
};

struct EdgeSerializationContext
{
    SymbolTablesBuilder* symbols;
};

struct GraphSerializationContext
{
    SymbolTablesBuilder symbols;

    auto state_context() noexcept { return StateSerializationContext { &symbols }; }
    auto edge_context() noexcept { return EdgeSerializationContext { &symbols }; }
};

struct DomainDeserializationContext
{
    tyr::formalism::planning::Parser* parser;
    tyr::ExecutionContextPtr execution_context;
};

template<tyr::planning::TaskKind Kind>
struct TaskDeserializationContext
{
    TaskSearchContext<Kind> search_context;
};

}  // namespace runir::datasets::serialization

#endif
