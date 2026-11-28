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
#include <tyr/planning/states/state_builder.hpp>
#include <tyr/planning/states/state_repository.hpp>
#include <tyr/planning/tasks/ground/task.hpp>
#include <tyr/planning/tasks/lifted/task.hpp>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/indexed.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/formalism/row_view.hpp>
#include <yggdrasil/formalism/symbol_context_impl.hpp>
#include <yggdrasil/formalism/symbol_context_instantiations.hpp>

namespace runir::datasets
{

namespace
{

class ObjectGraphConstructionContext
{
private:
    using ObjectView = tyr::formalism::ObjectView;
    using Edge = std::pair<VertexIndex, VertexIndex>;

    ColorRepository* m_repository;
    /// The objects by vertex: an object's vertex is its position.
    urd::Store<urd::set<ObjectView>, urd::positional> m_objects;
    urd::Store<urd::sequence<urd::flat_vector<ColorVariant>>> m_vertex_colors;
    urd::Store<urd::set<Edge>> m_edges;

    auto get_or_create_vertex(ObjectView object) -> VertexIndex
    {
        const auto [it, inserted] = m_objects.insert(object);
        if (inserted)
            m_vertex_colors.emplace_back();
        return static_cast<VertexIndex>(m_objects.position(it));
    }

    void add_undirected_edge(VertexIndex lhs, VertexIndex rhs)
    {
        if (lhs == rhs)
            return;

        if (rhs < lhs)
            std::swap(lhs, rhs);

        m_edges.emplace(lhs, rhs);
    }

public:
    explicit ObjectGraphConstructionContext(ColorRepository& repository) : m_repository(&repository), m_objects(urd::layer { 0 }) {}

    void add_object(ObjectView object) { static_cast<void>(get_or_create_vertex(object)); }

    template<PredicateContext Context, tyr::formalism::FactKind T>
    void add_atom(tyr::formalism::AtomView<::tyr::GroundTag, T> atom)
    {
        const auto objects = atom.row().template resolve<tyr::formalism::Object>();
        auto vertices = VertexIndexList {};
        vertices.reserve(objects.size());

        for (std::size_t i = 0; i < objects.size(); ++i)
        {
            const auto vertex = get_or_create_vertex(objects[i]);
            vertices.push_back(vertex);

            auto predicate_color_data = urd::data<PredicateColor<T>>(atom.predicate().index(), static_cast<ygg::uint_t>(i), Context);
            const auto predicate_color = m_repository->insert(predicate_color_data).first;
            m_vertex_colors[vertex].emplace_back(predicate_color.index());
        }

        for (std::size_t i = 0; i < vertices.size(); ++i)
            for (std::size_t j = i + 1; j < vertices.size(); ++j)
                add_undirected_edge(vertices[i], vertices[j]);
    }

    auto release() && -> std::unique_ptr<ObjectGraph>
    {
        auto vertices = urd::Store<urd::sequence<ColorView, VertexIndex>> {};
        vertices.reserve(m_vertex_colors.size());
        for (auto& colors : m_vertex_colors)
        {
            auto color_data = urd::data<Color>(std::move(colors));
            vertices.push_back(m_repository->insert(color_data).first);
        }

        auto edges = urd::Store<urd::sequence<Edge>> {};
        edges.reserve(2 * m_edges.size());
        for (const auto& [source, target] : m_edges)
        {
            edges.emplace_back(source, target);
            edges.emplace_back(target, source);
        }

        return std::make_unique<ObjectGraph>(urd::graph_data<Edge, ColorView> { vertices.size(), edges, {}, vertices });
    }
};

template<tyr::TaskKind Kind>
void add_objects(tyr::planning::StateBuilderView<Kind> state, ObjectGraphConstructionContext& context)
{
    const auto task = state.context().get_task()->get_task();

    for (auto object : task.domain().constants())
        context.add_object(object);

    for (auto object : task.objects())
        context.add_object(object);
}

template<tyr::TaskKind Kind>
void add_atoms(tyr::planning::StateBuilderView<Kind> state, ObjectGraphConstructionContext& context)
{
    for (auto atom : state.template atoms<tyr::formalism::StaticTag>())
        context.template add_atom<PredicateContext::STATE>(atom);

    for (const auto& fact : state.fluent_facts())
        if (const auto atom = fact.atom())
            context.template add_atom<PredicateContext::STATE>(*atom);
}

template<tyr::TaskKind Kind>
void add_goal_atoms(tyr::planning::StateBuilderView<Kind> state, ObjectGraphConstructionContext& context)
{
    const auto goal = state.context().get_task()->get_task().goal();

    for (auto literal : goal.template literals<tyr::formalism::StaticTag>())
        context.template add_atom<PredicateContext::GOAL>(literal.atom());

    for (auto fact : goal.template facts<tyr::formalism::PositiveTag>())
        if (const auto atom = fact.atom())
            context.template add_atom<PredicateContext::GOAL>(*atom);

    for (auto fact : goal.template facts<tyr::formalism::NegativeTag>())
        if (const auto atom = fact.atom())
            context.template add_atom<PredicateContext::GOAL>(*atom);
}

}  // namespace

template<tyr::TaskKind Kind>
auto create_object_graph(tyr::planning::StateBuilderView<Kind> state, ColorRepository& repository) -> std::unique_ptr<ObjectGraph>
{
    const auto root = [](const tyr::formalism::Repository* layer)
    {
        while (layer->parent() != nullptr)
            layer = layer->parent();
        return layer;
    };
    if (root(state.formalism_repository().get()) != root(&repository.get<tyr::formalism::Repository>()))
        throw std::invalid_argument("State and ColorRepository use different planning repositories.");

    auto context = ObjectGraphConstructionContext(repository);
    add_objects(state, context);
    add_atoms(state, context);
    add_goal_atoms(state, context);
    return std::move(context).release();
}

template auto create_object_graph<tyr::GroundTag>(tyr::planning::StateBuilderView<tyr::GroundTag>, ColorRepository&) -> std::unique_ptr<ObjectGraph>;

template auto create_object_graph<tyr::LiftedTag>(tyr::planning::StateBuilderView<tyr::LiftedTag>, ColorRepository&) -> std::unique_ptr<ObjectGraph>;

}  // namespace runir::datasets

// The color repository's symbols, which the library's other sources only declare.
YGG_FORMALISM_SYMBOL_CONTEXT(, runir::datasets::ColorRepository, 3)
