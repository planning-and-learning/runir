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

#include "runir/datasets/object_graph.hpp"

#include <cstddef>
#include <stdexcept>
#include <tyr/planning/ground/state_repository.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/state_repository.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>

namespace runir::datasets
{

namespace
{

class ObjectGraphConstructionContext
{
private:
    using ObjectView = tyr::formalism::planning::ObjectView;
    using Edge = std::pair<graphs::VertexIndex, graphs::VertexIndex>;

    ColorRepository& m_repository;
    ygg::UnorderedMap<ObjectView, graphs::VertexIndex> m_object_to_vertex;
    std::vector<ygg::Data<Color>::VariantList> m_vertex_colors;
    ygg::UnorderedSet<Edge> m_edges;

    auto get_or_create_vertex(ObjectView object) -> graphs::VertexIndex
    {
        if (const auto it = m_object_to_vertex.find(object); it != m_object_to_vertex.end())
            return it->second;

        const auto vertex = static_cast<graphs::VertexIndex>(m_vertex_colors.size());
        m_object_to_vertex.emplace(object, vertex);
        m_vertex_colors.emplace_back();
        return vertex;
    }

    void add_undirected_edge(graphs::VertexIndex lhs, graphs::VertexIndex rhs)
    {
        if (lhs == rhs)
            return;

        if (rhs < lhs)
            std::swap(lhs, rhs);

        m_edges.emplace(lhs, rhs);
    }

public:
    explicit ObjectGraphConstructionContext(ColorRepository& repository) : m_repository(repository) {}

    void add_object(ObjectView object) { static_cast<void>(get_or_create_vertex(object)); }

    template<PredicateContext Context, tyr::formalism::FactKind T>
    void add_atom(tyr::formalism::planning::GroundAtomView<T> atom)
    {
        const auto objects = atom.get_row().get_objects();
        auto vertices = std::vector<graphs::VertexIndex> {};
        vertices.reserve(objects.size());

        for (std::size_t i = 0; i < objects.size(); ++i)
        {
            const auto vertex = get_or_create_vertex(objects[i]);
            vertices.push_back(vertex);

            auto predicate_color_data = ygg::Data<PredicateColor<T>>(atom.get_predicate().get_index(), static_cast<ygg::uint_t>(i), Context);
            const auto predicate_color = m_repository.get_or_create(predicate_color_data).first;
            m_vertex_colors[vertex].emplace_back(predicate_color.get_index());
        }

        for (std::size_t i = 0; i < vertices.size(); ++i)
            for (std::size_t j = i + 1; j < vertices.size(); ++j)
                add_undirected_edge(vertices[i], vertices[j]);
    }

    auto release() && -> std::unique_ptr<ObjectGraph>
    {
        auto builder = ObjectGraphBuilder {};

        for (auto& colors : m_vertex_colors)
        {
            auto color_data = ygg::Data<Color>(std::move(colors));
            builder.add_vertex(m_repository.get_or_create(color_data).first);
        }

        for (const auto& [source, target] : m_edges)
            builder.add_undirected_edge(source, target);

        return std::make_unique<ObjectGraph>(std::move(builder));
    }
};

template<tyr::TaskKind Kind>
void add_objects(tyr::planning::StateView<Kind> state, ObjectGraphConstructionContext& context)
{
    const auto task = state.get_state_repository()->get_task()->get_task();

    for (auto object : task.get_domain().get_constants())
        context.add_object(object);

    for (auto object : task.get_objects())
        context.add_object(object);
}

template<tyr::TaskKind Kind>
void add_atoms(tyr::planning::StateView<Kind> state, ObjectGraphConstructionContext& context)
{
    for (auto atom : state.get_static_atoms_view())
        context.template add_atom<PredicateContext::STATE>(atom);

    for (auto fact : state.get_fluent_facts_view())
        if (const auto atom = fact.get_atom())
            context.template add_atom<PredicateContext::STATE>(*atom);
}

template<tyr::TaskKind Kind>
void add_goal_atoms(tyr::planning::StateView<Kind> state, ObjectGraphConstructionContext& context)
{
    const auto goal = state.get_state_repository()->get_task()->get_task().get_goal();

    for (auto literal : goal.template get_literals<tyr::formalism::StaticTag>())
        context.template add_atom<PredicateContext::GOAL>(literal.get_atom());

    for (auto fact : goal.template get_facts<tyr::formalism::PositiveTag>())
        if (const auto atom = fact.get_atom())
            context.template add_atom<PredicateContext::GOAL>(*atom);

    for (auto fact : goal.template get_facts<tyr::formalism::NegativeTag>())
        if (const auto atom = fact.get_atom())
            context.template add_atom<PredicateContext::GOAL>(*atom);
}

}  // namespace

template<tyr::TaskKind Kind>
auto create_object_graph(tyr::planning::StateView<Kind> state, ColorRepository& repository) -> std::unique_ptr<ObjectGraph>
{
    if (&state.get_repository()->get_root() != &repository.get_planning_repository().get_root())
        throw std::invalid_argument("State and ColorRepository use different planning repositories.");

    auto context = ObjectGraphConstructionContext(repository);
    add_objects(state, context);
    add_atoms(state, context);
    add_goal_atoms(state, context);
    return std::move(context).release();
}

template auto create_object_graph<tyr::GroundTag>(tyr::planning::StateView<tyr::GroundTag>, ColorRepository&) -> std::unique_ptr<ObjectGraph>;

template auto create_object_graph<tyr::LiftedTag>(tyr::planning::StateView<tyr::LiftedTag>, ColorRepository&) -> std::unique_ptr<ObjectGraph>;

}  // namespace runir::datasets
