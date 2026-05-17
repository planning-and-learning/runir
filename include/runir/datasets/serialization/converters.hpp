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

#ifndef RUNIR_DATASETS_SERIALIZATION_CONVERTERS_HPP_
#define RUNIR_DATASETS_SERIALIZATION_CONVERTERS_HPP_

#include "runir/datasets/serialization/archives.hpp"
#include "runir/datasets/state_graph.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <tyr/common/comparators.hpp>
#include <tyr/formalism/planning/repository.hpp>

namespace runir::datasets::serialization
{

template<typename Range>
auto object_names(Range&& range)
{
    auto result = std::vector<std::string> {};
    for (const auto object : range)
        result.push_back(std::string(object.get_name()));
    return result;
}

template<tyr::formalism::FactKind T>
auto archive(tyr::formalism::planning::GroundAtomView<T> atom) -> AtomArchive<T>
{
    return AtomArchive<T> { std::string(atom.get_predicate().get_name()), object_names(atom.get_row().get_objects()) };
}

template<tyr::formalism::FactKind T>
auto archive(tyr::formalism::planning::FDRFactView<T> fact) -> FDRFactArchive<T>
{
    if (const auto atom = fact.get_atom())
        return FDRFactArchive<T> { true, archive(atom.value()), {} };

    auto atoms = std::vector<AtomArchive<T>> {};
    for (const auto variable_atom : fact.get_variable().get_atoms())
        atoms.push_back(archive(variable_atom));
    std::sort(atoms.begin(), atoms.end(), tyr::Less<AtomArchive<T>> {});
    return FDRFactArchive<T> { false, {}, std::move(atoms) };
}

template<tyr::formalism::FactKind T>
auto archive(tyr::formalism::planning::GroundFunctionTermView<T> term) -> FunctionTermArchive<T>
{
    return FunctionTermArchive<T> { std::string(term.get_function().get_name()), object_names(term.get_row().get_objects()) };
}

template<tyr::formalism::FactKind T>
auto archive(const tyr::formalism::planning::GroundFunctionTermViewValuePair<T>& value) -> NumericValueArchive<T>
{
    const auto& [term, number] = value;
    return NumericValueArchive<T> { archive(term), number };
}

inline auto archive(tyr::formalism::planning::GroundActionView action) -> GroundActionArchive
{
    return GroundActionArchive { std::string(action.get_action().get_name()), object_names(action.get_row().get_objects()) };
}

inline auto archive(const StateGraphEdgeLabel& label) -> GroundActionArchive
{
    return archive(label.action);
}

}  // namespace runir::datasets::serialization

#endif
