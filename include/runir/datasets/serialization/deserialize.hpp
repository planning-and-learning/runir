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

#ifndef RUNIR_DATASETS_SERIALIZATION_DESERIALIZE_HPP_
#define RUNIR_DATASETS_SERIALIZATION_DESERIALIZE_HPP_

#include "runir/datasets/serialization/context.hpp"
#include "runir/datasets/serialization/converters.hpp"
#include "runir/datasets/state_graph.hpp"
#include "runir/datasets/task_class.hpp"

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <tyr/common/declarations.hpp>
#include <tyr/common/types.hpp>
#include <tyr/formalism/binding_data.hpp>
#include <tyr/formalism/planning/fdr_fact_data.hpp>
#include <tyr/formalism/planning/fdr_value.hpp>
#include <tyr/formalism/planning/parser.hpp>
#include <vector>

namespace runir::datasets::serialization
{

namespace detail
{

template<typename Lookup, typename Key>
auto lookup_required(const Lookup& lookup, const Key& key, const std::string& what)
{
    const auto it = lookup.find(key);
    if (it == lookup.end())
        throw std::runtime_error("Could not deserialize " + what + ".");
    return it->second;
}

template<tyr::planning::TaskKind Kind>
auto get_object(const TaskSearchContext<Kind>& context, const std::string& name) -> tyr::Index<tyr::formalism::Object>
{
    for (auto object : context.task->get_task().get_objects())
    {
        if (object.get_name() == name)
            return object.get_index();
    }

    for (auto object : context.task->get_domain().get_domain().get_constants())
    {
        if (object.get_name() == name)
            return object.get_index();
    }

    throw std::runtime_error("Could not deserialize object: " + name + ".");
}

template<tyr::formalism::FactKind T>
using FunctionTermValueLookup = tyr::UnorderedMap<FunctionTermArchive<T>, tyr::formalism::planning::GroundFunctionTermViewValuePair<T>>;

using FDRFactLookup = tyr::UnorderedMap<FDRFactArchive<tyr::formalism::FluentTag>, tyr::formalism::planning::FDRFactView<tyr::formalism::FluentTag>>;

template<tyr::planning::TaskKind Kind, tyr::formalism::FactKind T>
auto make_function_term_value_lookup(const TaskSearchContext<Kind>& context)
{
    auto result = FunctionTermValueLookup<T> {};
    for (auto term_value : context.task->get_task().template get_fterm_values<T>())
        result.emplace(serialization::archive(term_value.get_fterm()),
                       tyr::formalism::planning::GroundFunctionTermViewValuePair<T> { term_value.get_fterm(), term_value.get_value() });
    return result;
}

template<tyr::planning::TaskKind Kind>
auto make_fdr_fact_lookup(const TaskSearchContext<Kind>& context)
{
    namespace f = tyr::formalism;
    namespace fp = tyr::formalism::planning;

    auto result = FDRFactLookup {};

    for (auto variable : context.task->get_fdr_context()->get_variables())
    {
        for (auto atom : variable.get_atoms())
        {
            const auto fact = context.task->get_fdr_context()->get_fact_view(atom);
            result.emplace(serialization::archive(fact), fact);
        }

        auto none_of_atoms = std::vector<AtomArchive<f::FluentTag>> {};
        for (auto atom : variable.get_atoms())
            none_of_atoms.push_back(serialization::archive(atom));
        std::sort(none_of_atoms.begin(), none_of_atoms.end(), tyr::Less<AtomArchive<f::FluentTag>> {});

        const auto fact = tyr::make_view(tyr::Data<fp::FDRFact<f::FluentTag>>(variable.get_index(), fp::FDRValue::none()), *context.task->get_repository());
        result.emplace(FDRFactArchive<f::FluentTag> { false, {}, std::move(none_of_atoms) }, fact);
    }

    return result;
}

template<tyr::planning::TaskKind Kind>
auto deserialize_action_binding(TaskSearchContext<Kind>& context, const GroundActionArchive& archive) -> tyr::formalism::planning::ActionBindingView
{
    namespace fp = tyr::formalism::planning;

    auto action = tyr::Index<fp::Action> {};
    auto found_action = false;
    for (auto candidate : context.task->get_domain().get_domain().get_actions())
    {
        if (tyr::uint_t(candidate.get_index()) == archive.action_index && candidate.get_name() == archive.action)
        {
            action = candidate.get_index();
            found_action = true;
            break;
        }
    }
    if (!found_action)
        throw std::runtime_error("Could not deserialize action " + archive.action + "/" + std::to_string(archive.action_index) + ".");

    auto objects = tyr::IndexList<tyr::formalism::Object> {};
    objects.reserve(archive.objects.size());
    for (const auto& object : archive.objects)
        objects.push_back(get_object(context, object));

    const auto binding = tyr::Data<tyr::formalism::RelationBinding<fp::Action>>(action, objects.size(), std::move(objects));
    return context.task->get_repository()->get_or_create(binding).first;
}

template<tyr::planning::TaskKind Kind>
auto make_ground_action_lookup(TaskSearchContext<Kind>& context, const SymbolTableArchive<GroundActionArchive>& symbols)
{
    auto result = std::vector<tyr::formalism::planning::GroundActionView> {};
    result.reserve(symbols.symbols.size());

    if constexpr (std::same_as<Kind, tyr::planning::GroundTag>)
    {
        auto lookup = tyr::UnorderedMap<GroundActionArchive, tyr::formalism::planning::GroundActionView> {};
        for (auto action : context.task->get_task().get_ground_actions())
            lookup.emplace(serialization::archive(action), action);

        for (const auto& symbol : symbols.symbols)
            result.push_back(lookup_required(lookup, symbol, "ground action " + symbol.action));
    }
    else
    {
        for (const auto& symbol : symbols.symbols)
        {
            try
            {
                const auto binding = deserialize_action_binding(context, symbol);
                result.push_back(context.successor_generator->get_ground_action(binding));
            }
            catch (const std::exception& error)
            {
                throw std::runtime_error("Could not deserialize lifted ground action " + symbol.action + "/" + std::to_string(symbol.objects.size()) + ": "
                                         + error.what());
            }
        }
    }

    return result;
}

struct ResolvedSymbols
{
    std::vector<tyr::formalism::planning::FDRFactView<tyr::formalism::FluentTag>> fluent_facts;
    std::vector<tyr::formalism::planning::GroundFunctionTermViewValuePair<tyr::formalism::FluentTag>> fluent_numeric_values;
    std::vector<tyr::formalism::planning::GroundActionView> ground_actions;
};

template<tyr::planning::TaskKind Kind>
auto resolve_symbols(TaskSearchContext<Kind>& context, const SymbolTablesArchive& symbols) -> ResolvedSymbols
{
    namespace f = tyr::formalism;

    const auto fluent_facts = make_fdr_fact_lookup(context);
    const auto fluent_numeric_values = make_function_term_value_lookup<Kind, f::FluentTag>(context);

    auto result = ResolvedSymbols {};

    result.fluent_facts.reserve(symbols.fluent_facts.symbols.size());
    for (const auto& symbol : symbols.fluent_facts.symbols)
        result.fluent_facts.push_back(lookup_required(fluent_facts, symbol, "fluent fact"));

    result.fluent_numeric_values.reserve(symbols.fluent_numeric_values.symbols.size());
    for (const auto& symbol : symbols.fluent_numeric_values.symbols)
    {
        auto value = lookup_required(fluent_numeric_values, symbol.term, "fluent numeric value " + symbol.term.function);
        value.second = symbol.value;
        result.fluent_numeric_values.push_back(value);
    }

    result.ground_actions = make_ground_action_lookup(context, symbols.ground_actions);

    return result;
}

template<tyr::planning::TaskKind Kind>
auto deserialize_state(TaskSearchContext<Kind>& context, const StateArchive& archive, const ResolvedSymbols& symbols) -> tyr::planning::StateView<Kind>
{
    namespace f = tyr::formalism;

    auto fluent_facts = std::vector<tyr::formalism::planning::FDRFactView<f::FluentTag>> {};
    fluent_facts.reserve(archive.fluent_facts.size());
    for (const auto symbol : archive.fluent_facts)
        fluent_facts.push_back(symbols.fluent_facts.at(symbol));

    auto fluent_numeric_values = std::vector<tyr::formalism::planning::GroundFunctionTermViewValuePair<f::FluentTag>> {};
    fluent_numeric_values.reserve(archive.fluent_numeric_values.size());
    for (const auto symbol : archive.fluent_numeric_values)
        fluent_numeric_values.push_back(symbols.fluent_numeric_values.at(symbol));

    return context.state_repository->create_state(fluent_facts, fluent_numeric_values);
}

inline auto deserialize_edge_label(const EdgeArchive& archive, const ResolvedSymbols& symbols) -> StateGraphEdgeLabel
{
    return StateGraphEdgeLabel { symbols.ground_actions.at(archive.action), archive.cost };
}

}  // namespace detail

template<tyr::planning::TaskKind Kind>
auto deserialize(DomainDeserializationContext context, const StateGraphArchive& archive) -> TaskDeserializationContext<Kind>
{
    namespace p = tyr::planning;

    auto planning_task = context.parser->parse_task(archive.problem, std::filesystem::path("<serialized-problem>"));

    if constexpr (std::same_as<Kind, p::GroundTag>)
    {
        auto lifted_task = p::Task<p::LiftedTag>(planning_task);
        auto task = lifted_task.instantiate_ground_task(*context.execution_context).task;
        return TaskDeserializationContext<p::GroundTag> { TaskSearchContext<p::GroundTag>(std::move(task), std::move(context.execution_context)) };
    }
    else if constexpr (std::same_as<Kind, p::LiftedTag>)
    {
        auto task = p::Task<p::LiftedTag>::create(std::move(planning_task));
        return TaskDeserializationContext<p::LiftedTag> { TaskSearchContext<p::LiftedTag>(std::move(task), std::move(context.execution_context)) };
    }
}

template<tyr::planning::TaskKind Kind>
auto deserialize(tyr::formalism::planning::Parser& parser,
                 const StateGraphArchive& archive,
                 tyr::ExecutionContextPtr execution_context) -> TaskSearchContext<Kind>
{
    auto context = deserialize<Kind>(DomainDeserializationContext { &parser, std::move(execution_context) }, archive);
    return std::move(context.search_context);
}

template<tyr::planning::TaskKind Kind>
auto deserialize(DomainDeserializationContext context, const AnnotatedStateGraphArchive& archive) -> TaskDeserializationContext<Kind>
{
    auto state_graph_archive = StateGraphArchive {};
    state_graph_archive.problem = archive.problem;
    return deserialize<Kind>(context, state_graph_archive);
}

template<tyr::planning::TaskKind Kind>
auto deserialize(tyr::formalism::planning::Parser& parser,
                 const AnnotatedStateGraphArchive& archive,
                 tyr::ExecutionContextPtr execution_context) -> TaskSearchContext<Kind>
{
    auto context = deserialize<Kind>(DomainDeserializationContext { &parser, std::move(execution_context) }, archive);
    return std::move(context.search_context);
}

template<tyr::planning::TaskKind Kind>
auto deserialize(TaskSearchContext<Kind>& context, const StateGraphArchive& archive) -> std::unique_ptr<StateGraph<Kind>>
{
    auto builder = StateGraphBuilder<Kind> {};
    const auto symbols = detail::resolve_symbols(context, archive.symbols);

    for (const auto& state : archive.states)
        builder.add_vertex(StateGraphVertexLabel<Kind> { detail::deserialize_state(context, state, symbols) });

    for (const auto& edge : archive.edges)
        builder.add_directed_edge(edge.source, edge.target, detail::deserialize_edge_label(edge, symbols));

    return std::make_unique<StateGraph<Kind>>(std::move(builder));
}

template<tyr::planning::TaskKind Kind>
auto deserialize(TaskSearchContext<Kind>& context, const AnnotatedStateGraphArchive& archive) -> std::unique_ptr<AnnotatedStateGraph<Kind>>
{
    auto builder = AnnotatedStateGraphBuilder<Kind> {};
    const auto symbols = detail::resolve_symbols(context, archive.symbols);

    for (const auto& state : archive.states)
    {
        builder.add_vertex(AnnotatedStateGraphVertexLabel<Kind> { detail::deserialize_state(context, state.state, symbols),
                                                                  state.goal_distance,
                                                                  state.is_initial,
                                                                  state.is_goal,
                                                                  state.is_alive,
                                                                  state.is_unsolvable });
    }

    for (const auto& edge : archive.edges)
        builder.add_directed_edge(edge.source, edge.target, detail::deserialize_edge_label(edge, symbols));

    return std::make_unique<AnnotatedStateGraph<Kind>>(std::move(builder));
}

}  // namespace runir::datasets::serialization

#endif
