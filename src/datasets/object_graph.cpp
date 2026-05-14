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

#include <algorithm>
#include <cassert>
#include <tyr/common/comparators.hpp>
#include <tyr/common/equal_to.hpp>
#include <tyr/common/declarations.hpp>
#include <utility>

namespace runir::datasets
{

namespace
{

template<tyr::planning::TaskKind Kind>
class ObjectGraphConstructionContext
{
private:
    using ObjectView = tyr::formalism::planning::ObjectView;
    using Edge = std::pair<graphs::VertexIndex, graphs::VertexIndex>;

    tyr::UnorderedMap<ObjectView, graphs::VertexIndex> m_object_to_vertex;
    std::vector<ObjectGraphVertexLabel> m_vertex_labels;
    tyr::UnorderedSet<Edge> m_edges;

    auto get_or_create_vertex(ObjectView object) -> graphs::VertexIndex
    {
        if (const auto it = m_object_to_vertex.find(object); it != m_object_to_vertex.end())
            return it->second;

        const auto vertex = static_cast<graphs::VertexIndex>(m_vertex_labels.size());
        m_object_to_vertex.emplace(object, vertex);
        m_vertex_labels.emplace_back();
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
    void add_object(ObjectView object) { static_cast<void>(get_or_create_vertex(object)); }

    template<tyr::formalism::FactKind FactKind>
    void add_atom(tyr::formalism::planning::GroundAtomView<FactKind> atom, ObjectGraphPredicateKind kind)
    {
        const auto predicate = ObjectGraphPredicateVariant(atom.get_predicate());
        const auto objects = atom.get_row().get_objects();
        auto vertices = std::vector<graphs::VertexIndex> {};
        vertices.reserve(objects.size());

        for (std::size_t i = 0; i < objects.size(); ++i)
        {
            const auto vertex = get_or_create_vertex(objects[i]);
            vertices.push_back(vertex);
            m_vertex_labels[vertex].labels.push_back(ObjectGraphVertexLabelEntry { predicate, i, kind });
        }

        for (std::size_t i = 0; i < vertices.size(); ++i)
            for (std::size_t j = i + 1; j < vertices.size(); ++j)
                add_undirected_edge(vertices[i], vertices[j]);
    }

    auto release() && -> std::unique_ptr<ObjectGraph<Kind>>
    {
        auto builder = ObjectGraphBuilder<Kind> {};

        for (auto& label : m_vertex_labels)
        {
            std::sort(label.labels.begin(), label.labels.end(), tyr::Less<ObjectGraphVertexLabelEntry> {});
            label.labels.erase(std::unique(label.labels.begin(), label.labels.end(), tyr::EqualTo<ObjectGraphVertexLabelEntry> {}), label.labels.end());
            builder.add_vertex(std::move(label));
        }

        for (const auto& [source, target] : m_edges)
        {
            if (source == target)
                builder.add_directed_edge(source, target);
            else
                builder.add_undirected_edge(source, target);
        }

        return std::make_unique<ObjectGraph<Kind>>(std::move(builder));
    }
};

template<tyr::planning::TaskKind Kind>
void add_objects(tyr::planning::StateView<Kind> state, ObjectGraphConstructionContext<Kind>& context)
{
    const auto task = state.get_state_repository()->get_task()->get_task();

    for (auto object : task.get_domain().get_constants())
        context.add_object(object);

    for (auto object : task.get_objects())
        context.add_object(object);
}

template<tyr::planning::TaskKind Kind>
void add_atoms(tyr::planning::StateView<Kind> state, ObjectGraphConstructionContext<Kind>& context)
{
    for (auto atom : state.get_static_atoms_view())
        context.add_atom(atom, ObjectGraphPredicateKind::STATE_PREDICATE);

    for (auto fact : state.get_fluent_facts_view())
        if (const auto atom = fact.get_atom())
            context.add_atom(*atom, ObjectGraphPredicateKind::STATE_PREDICATE);
}

template<tyr::planning::TaskKind Kind>
void add_goal_atoms(tyr::planning::StateView<Kind> state, ObjectGraphConstructionContext<Kind>& context)
{
    const auto goal = state.get_state_repository()->get_task()->get_task().get_goal();

    for (auto literal : goal.template get_literals<tyr::formalism::StaticTag>())
        context.add_atom(literal.get_atom(), ObjectGraphPredicateKind::GOAL_PREDICATE);

    for (auto fact : goal.template get_facts<tyr::formalism::PositiveTag>())
        if (const auto atom = fact.get_atom())
            context.add_atom(*atom, ObjectGraphPredicateKind::GOAL_PREDICATE);

    for (auto fact : goal.template get_facts<tyr::formalism::NegativeTag>())
        if (const auto atom = fact.get_atom())
            context.add_atom(*atom, ObjectGraphPredicateKind::GOAL_PREDICATE);
}

}  // namespace

template<tyr::planning::TaskKind Kind>
auto create_object_graph(tyr::planning::StateView<Kind> state) -> std::unique_ptr<ObjectGraph<Kind>>
{
    auto context = ObjectGraphConstructionContext<Kind> {};
    add_objects(state, context);
    add_atoms(state, context);
    add_goal_atoms(state, context);
    return std::move(context).release();
}

template auto create_object_graph<tyr::planning::GroundTag>(tyr::planning::StateView<tyr::planning::GroundTag>)
    -> std::unique_ptr<ObjectGraph<tyr::planning::GroundTag>>;

template auto create_object_graph<tyr::planning::LiftedTag>(tyr::planning::StateView<tyr::planning::LiftedTag>)
    -> std::unique_ptr<ObjectGraph<tyr::planning::LiftedTag>>;

}  // namespace runir::datasets
