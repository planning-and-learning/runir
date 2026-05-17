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

#ifndef RUNIR_DATASETS_SERIALIZATION_ARCHIVES_HPP_
#define RUNIR_DATASETS_SERIALIZATION_ARCHIVES_HPP_

#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <tyr/formalism/declarations.hpp>
#include <tyr/common/types.hpp>

namespace runir::datasets::serialization
{

using SymbolIndex = std::size_t;

template<tyr::formalism::FactKind T>
struct AtomArchive
{
    std::string predicate;
    std::vector<std::string> objects;

    auto identifying_members() const noexcept { return std::tie(predicate, objects); }
};

template<tyr::formalism::FactKind T>
struct FDRFactArchive
{
    bool has_atom = false;
    AtomArchive<T> atom;
    std::vector<AtomArchive<T>> none_of_atoms;

    auto identifying_members() const noexcept { return std::tie(has_atom, atom, none_of_atoms); }
};

template<tyr::formalism::FactKind T>
struct FunctionTermArchive
{
    std::string function;
    std::vector<std::string> objects;

    auto identifying_members() const noexcept { return std::tie(function, objects); }
};

template<tyr::formalism::FactKind T>
struct NumericValueArchive
{
    FunctionTermArchive<T> term;
    tyr::float_t value = 0;

    auto identifying_members() const noexcept { return std::tie(term, value); }
};

struct GroundActionArchive
{
    std::string action;
    std::vector<std::string> objects;

    auto identifying_members() const noexcept { return std::tie(action, objects); }
};

template<typename T>
struct SymbolTableArchive
{
    std::vector<T> symbols;
};

struct SymbolTablesArchive
{
    SymbolTableArchive<AtomArchive<tyr::formalism::StaticTag>> static_atoms;
    SymbolTableArchive<FDRFactArchive<tyr::formalism::FluentTag>> fluent_facts;
    SymbolTableArchive<AtomArchive<tyr::formalism::DerivedTag>> derived_atoms;
    SymbolTableArchive<NumericValueArchive<tyr::formalism::StaticTag>> static_numeric_values;
    SymbolTableArchive<NumericValueArchive<tyr::formalism::FluentTag>> fluent_numeric_values;
    SymbolTableArchive<GroundActionArchive> ground_actions;
};

struct StateArchive
{
    std::vector<SymbolIndex> static_atoms;
    std::vector<SymbolIndex> fluent_facts;
    std::vector<SymbolIndex> derived_atoms;
    std::vector<SymbolIndex> static_numeric_values;
    std::vector<SymbolIndex> fluent_numeric_values;
};

struct AnnotatedStateArchive
{
    StateArchive state;
    tyr::float_t goal_distance = 0;
    bool is_initial = false;
    bool is_goal = false;
    bool is_alive = false;
    bool is_unsolvable = false;
};

struct EdgeArchive
{
    std::size_t source = 0;
    std::size_t target = 0;
    SymbolIndex action = 0;
    tyr::float_t cost = 0;
};

struct StateGraphArchive
{
    std::string problem;
    SymbolTablesArchive symbols;
    std::vector<StateArchive> states;
    std::vector<EdgeArchive> edges;
};

struct AnnotatedStateGraphArchive
{
    std::string problem;
    SymbolTablesArchive symbols;
    std::vector<AnnotatedStateArchive> states;
    std::vector<EdgeArchive> edges;
};

}  // namespace runir::datasets::serialization

namespace boost::serialization
{

template<class Archive, tyr::formalism::FactKind T>
void serialize(Archive& archive, runir::datasets::serialization::AtomArchive<T>& value, const unsigned int)
{
    archive & make_nvp("predicate", value.predicate);
    archive & make_nvp("objects", value.objects);
}

template<class Archive, tyr::formalism::FactKind T>
void serialize(Archive& archive, runir::datasets::serialization::FDRFactArchive<T>& value, const unsigned int)
{
    archive & make_nvp("has_atom", value.has_atom);
    archive & make_nvp("atom", value.atom);
    archive & make_nvp("none_of_atoms", value.none_of_atoms);
}

template<class Archive, tyr::formalism::FactKind T>
void serialize(Archive& archive, runir::datasets::serialization::FunctionTermArchive<T>& value, const unsigned int)
{
    archive & make_nvp("function", value.function);
    archive & make_nvp("objects", value.objects);
}

template<class Archive, tyr::formalism::FactKind T>
void serialize(Archive& archive, runir::datasets::serialization::NumericValueArchive<T>& value, const unsigned int)
{
    archive & make_nvp("term", value.term);
    archive & make_nvp("value", value.value);
}

template<class Archive>
void serialize(Archive& archive, runir::datasets::serialization::GroundActionArchive& value, const unsigned int)
{
    archive & make_nvp("action", value.action);
    archive & make_nvp("objects", value.objects);
}

template<class Archive, typename T>
void serialize(Archive& archive, runir::datasets::serialization::SymbolTableArchive<T>& value, const unsigned int)
{
    archive & make_nvp("symbols", value.symbols);
}

template<class Archive>
void serialize(Archive& archive, runir::datasets::serialization::SymbolTablesArchive& value, const unsigned int)
{
    archive & make_nvp("static_atoms", value.static_atoms);
    archive & make_nvp("fluent_facts", value.fluent_facts);
    archive & make_nvp("derived_atoms", value.derived_atoms);
    archive & make_nvp("static_numeric_values", value.static_numeric_values);
    archive & make_nvp("fluent_numeric_values", value.fluent_numeric_values);
    archive & make_nvp("ground_actions", value.ground_actions);
}

template<class Archive>
void serialize(Archive& archive, runir::datasets::serialization::StateArchive& value, const unsigned int)
{
    archive & make_nvp("static_atoms", value.static_atoms);
    archive & make_nvp("fluent_facts", value.fluent_facts);
    archive & make_nvp("derived_atoms", value.derived_atoms);
    archive & make_nvp("static_numeric_values", value.static_numeric_values);
    archive & make_nvp("fluent_numeric_values", value.fluent_numeric_values);
}

template<class Archive>
void serialize(Archive& archive, runir::datasets::serialization::AnnotatedStateArchive& value, const unsigned int)
{
    archive & make_nvp("state", value.state);
    archive & make_nvp("goal_distance", value.goal_distance);
    archive & make_nvp("is_initial", value.is_initial);
    archive & make_nvp("is_goal", value.is_goal);
    archive & make_nvp("is_alive", value.is_alive);
    archive & make_nvp("is_unsolvable", value.is_unsolvable);
}

template<class Archive>
void serialize(Archive& archive, runir::datasets::serialization::EdgeArchive& value, const unsigned int)
{
    archive & make_nvp("source", value.source);
    archive & make_nvp("target", value.target);
    archive & make_nvp("action", value.action);
    archive & make_nvp("cost", value.cost);
}

template<class Archive>
void serialize(Archive& archive, runir::datasets::serialization::StateGraphArchive& value, const unsigned int)
{
    archive & make_nvp("problem", value.problem);
    archive & make_nvp("symbols", value.symbols);
    archive & make_nvp("states", value.states);
    archive & make_nvp("edges", value.edges);
}

template<class Archive>
void serialize(Archive& archive, runir::datasets::serialization::AnnotatedStateGraphArchive& value, const unsigned int)
{
    archive & make_nvp("problem", value.problem);
    archive & make_nvp("symbols", value.symbols);
    archive & make_nvp("states", value.states);
    archive & make_nvp("edges", value.edges);
}

}  // namespace boost::serialization

#endif
