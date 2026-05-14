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

#include "runir/datasets/equivalence_graph.hpp"

#include "runir/datasets/equivalence_policies/gi.hpp"
#include "runir/datasets/equivalence_policies/identity.hpp"

#include <cassert>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tyr/common/equal_to.hpp>
#include <utility>
#include <vector>

namespace runir::datasets
{

namespace
{

template<tyr::planning::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
class EquivalenceGraphEventHandler : public tyr::planning::astar_eager::EventHandler<Kind>
{
private:
    using StateToVertexMap = tyr::UnorderedMap<tyr::planning::StateView<Kind>, graphs::VertexIndex>;
    using StateToEquivalenceVertexMap = tyr::UnorderedMap<tyr::planning::StateView<Kind>, graphs::VertexIndex>;
    using RepresentativeToVertexMap = tyr::UnorderedMap<EquivalenceVertexLabel, graphs::VertexIndex>;
    using EquivalenceEdgeSet = tyr::UnorderedSet<std::pair<graphs::VertexIndex, graphs::VertexIndex>>;

    uint_t m_state_graph_index;
    Policy* m_policy;
    EquivalenceGraphBuilder* m_equivalence_builder;
    RepresentativeToVertexMap* m_representative_to_vertex;
    EquivalenceEdgeSet* m_equivalence_edges;

    StateGraphBuilder<Kind> m_state_builder;
    StateToVertexMap m_state_to_vertex;
    tyr::UnorderedMap<tyr::planning::StateView<Kind>, StateGraphVertexRef> m_pruned_state_to_representative;
    std::vector<graphs::VertexIndex> m_state_vertex_to_equivalence_vertex;
    tyr::planning::Statistics m_statistics;

    auto get_or_create_equivalence_vertex(StateGraphVertexRef representative) -> graphs::VertexIndex
    {
        const auto label = EquivalenceVertexLabel { representative.state_graph_index, representative.state_vertex_index };
        if (const auto it = m_representative_to_vertex->find(label); it != m_representative_to_vertex->end())
            return it->second;

        const auto vertex = static_cast<graphs::VertexIndex>(m_representative_to_vertex->size());
        m_representative_to_vertex->emplace(label, vertex);
        [[maybe_unused]] const auto added = m_equivalence_builder->add_vertex(label);
        assert(added == vertex);
        return vertex;
    }

    auto next_state_vertex() const noexcept -> graphs::VertexIndex { return static_cast<graphs::VertexIndex>(m_state_to_vertex.size()); }

    auto get_or_create_state_vertex(tyr::planning::StateView<Kind> state) -> graphs::VertexIndex
    {
        if (const auto it = m_state_to_vertex.find(state); it != m_state_to_vertex.end())
            return it->second;

        const auto state_vertex = next_state_vertex();
        m_state_to_vertex.emplace(state, state_vertex);
        [[maybe_unused]] const auto added = m_state_builder.add_vertex(StateGraphVertexLabel<Kind> { state });
        assert(added == state_vertex);

        const auto representative =
            m_policy->get_or_create_representative(StateGraphVertexCandidate<Kind> { m_state_graph_index, state },
                                                   StateGraphVertexRef { m_state_graph_index, state_vertex });
        const auto equivalence_vertex = get_or_create_equivalence_vertex(representative);

        assert(m_state_vertex_to_equivalence_vertex.size() == state_vertex);
        m_state_vertex_to_equivalence_vertex.push_back(equivalence_vertex);

        return state_vertex;
    }

    auto get_or_create_state_vertex(tyr::planning::Node<Kind> node) -> graphs::VertexIndex { return get_or_create_state_vertex(node.get_state()); }

    auto get_equivalence_vertex(tyr::planning::StateView<Kind> state) -> graphs::VertexIndex
    {
        if (const auto it = m_state_to_vertex.find(state); it != m_state_to_vertex.end())
            return m_state_vertex_to_equivalence_vertex[it->second];

        const auto it = m_pruned_state_to_representative.find(state);
        assert(it != m_pruned_state_to_representative.end());
        return get_or_create_equivalence_vertex(it->second);
    }

    void record_edge(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node)
    {
        const auto source = get_or_create_state_vertex(source_node);
        const auto cost = labeled_succ_node.node.get_metric() - source_node.get_metric();

        auto state_edge = graphs::EdgeIndex {};
        if (const auto target_it = m_state_to_vertex.find(labeled_succ_node.node.get_state()); target_it != m_state_to_vertex.end())
        {
            state_edge = m_state_builder.add_directed_edge(source, target_it->second, StateGraphEdgeLabel { labeled_succ_node.label, cost });
        }
        else
        {
            const auto representative_it = m_pruned_state_to_representative.find(labeled_succ_node.node.get_state());
            assert(representative_it != m_pruned_state_to_representative.end());
            if (representative_it->second.state_graph_index == m_state_graph_index)
                state_edge = m_state_builder.add_directed_edge(source,
                                                               representative_it->second.state_vertex_index,
                                                               StateGraphEdgeLabel { labeled_succ_node.label, cost });
            else
                assert(false && "Cannot create a concrete state edge to a representative in a different state graph.");
        }

        const auto target_state = labeled_succ_node.node.get_state();
        const auto equivalence_source = m_state_vertex_to_equivalence_vertex[source];
        const auto equivalence_target = get_equivalence_vertex(target_state);
        if (m_equivalence_edges->emplace(equivalence_source, equivalence_target).second)
            m_equivalence_builder->add_directed_edge(equivalence_source,
                                                     equivalence_target,
                                                     EquivalenceEdgeLabel { m_state_graph_index, state_edge });
    }

public:
    EquivalenceGraphEventHandler(uint_t state_graph_index,
                                 Policy& policy,
                                 EquivalenceGraphBuilder& equivalence_builder,
                                 RepresentativeToVertexMap& representative_to_vertex,
                                 EquivalenceEdgeSet& equivalence_edges) :
        m_state_graph_index(state_graph_index),
        m_policy(&policy),
        m_equivalence_builder(&equivalence_builder),
        m_representative_to_vertex(&representative_to_vertex),
        m_equivalence_edges(&equivalence_edges)
    {
    }

    auto should_prune_successor_state(const tyr::planning::StateView<Kind>& state, const tyr::planning::StateView<Kind>& succ_state, bool is_new_succ) -> bool
    {
        static_cast<void>(state);

        if (!is_new_succ)
            return false;

        const auto proposed_representative = StateGraphVertexRef { m_state_graph_index, next_state_vertex() };
        const auto representative =
            m_policy->get_or_create_representative(StateGraphVertexCandidate<Kind> { m_state_graph_index, succ_state }, proposed_representative);

        if (tyr::EqualTo<StateGraphVertexRef> {}(representative, proposed_representative))
        {
            static_cast<void>(get_or_create_state_vertex(succ_state));
            return false;
        }

        m_pruned_state_to_representative.emplace(succ_state, representative);
        return true;
    }

    void on_expand_node(const tyr::planning::Node<Kind>& node) override { get_or_create_state_vertex(node); }

    void on_expand_goal_node(const tyr::planning::Node<Kind>& node) override { static_cast<void>(node); }

    void on_generate_node(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node) override
    {
        record_edge(source_node, labeled_succ_node);
    }

    void on_generate_node_relaxed(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node) override
    {
        static_cast<void>(source_node);
        static_cast<void>(labeled_succ_node);
    }

    void on_generate_node_not_relaxed(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node) override
    {
        record_edge(source_node, labeled_succ_node);
    }

    void on_close_node(const tyr::planning::Node<Kind>& node) override { static_cast<void>(node); }

    void on_prune_node(const tyr::planning::Node<Kind>& node) override { static_cast<void>(node); }

    void on_prune_node(const tyr::planning::Node<Kind>& source_node, const tyr::planning::LabeledNode<Kind>& labeled_succ_node) override
    {
        record_edge(source_node, labeled_succ_node);
    }

    void on_start_search(const tyr::planning::Node<Kind>& node, tyr::float_t f_value) override
    {
        static_cast<void>(f_value);
        get_or_create_state_vertex(node);
    }

    void on_finish_f_layer(tyr::float_t f_value) override { static_cast<void>(f_value); }

    void on_end_search() {}

    void on_end_search(tyr::planning::SearchStatus status) { static_cast<void>(status); }

    void on_solved(const tyr::planning::Plan<Kind>& plan) override { static_cast<void>(plan); }

    void on_unsolvable() {}

    void on_exhausted() {}

    const tyr::planning::Statistics& get_search_statistics() const { return m_statistics; }

    const tyr::planning::Statistics& get_statistics() const { return m_statistics; }

    auto release() && { return std::make_unique<StateGraph<Kind>>(std::move(m_state_builder)); }
};

template<tyr::planning::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
class EquivalenceGraphPruningStrategy : public tyr::planning::PruningStrategy<Kind>
{
private:
    EquivalenceGraphEventHandler<Kind, Policy>* m_event_handler;

public:
    explicit EquivalenceGraphPruningStrategy(EquivalenceGraphEventHandler<Kind, Policy>& event_handler) : m_event_handler(&event_handler) {}

    bool should_prune_successor_state(const tyr::planning::StateView<Kind>& state,
                                      const tyr::planning::StateView<Kind>& succ_state,
                                      bool is_new_succ) override
    {
        return m_event_handler->should_prune_successor_state(state, succ_state, is_new_succ);
    }
};

}  // namespace

template<tyr::planning::TaskKind Kind, IsEquivalencePolicy<Kind> Policy>
auto generate_equivalence_graph(TaskSearchContextList<Kind>& contexts, Policy& policy) -> EquivalenceGraphConstructionResult<Kind>
{
    auto equivalence_builder = EquivalenceGraphBuilder {};
    auto representative_to_vertex = tyr::UnorderedMap<EquivalenceVertexLabel, graphs::VertexIndex> {};
    auto equivalence_edges = tyr::UnorderedSet<std::pair<graphs::VertexIndex, graphs::VertexIndex>> {};
    auto state_graphs = std::vector<std::unique_ptr<StateGraph<Kind>>> {};
    state_graphs.reserve(contexts.size());

    for (uint_t state_graph_index = 0; state_graph_index < contexts.size(); ++state_graph_index)
    {
        auto& context = contexts[state_graph_index];
        auto heuristic = tyr::planning::BlindHeuristic<Kind> {};
        auto event_handler =
            std::make_shared<EquivalenceGraphEventHandler<Kind, Policy>>(state_graph_index, policy, equivalence_builder, representative_to_vertex, equivalence_edges);
        auto options = tyr::planning::astar_eager::Options<Kind> {};
        options.event_handler = event_handler;
        options.goal_strategy = tyr::planning::ExhaustiveGoalStrategy<Kind>::create();
        options.pruning_strategy = std::make_shared<EquivalenceGraphPruningStrategy<Kind, Policy>>(*event_handler);

        [[maybe_unused]] const auto result = tyr::planning::astar_eager::find_solution(*context.task, *context.successor_generator, heuristic, options);

        state_graphs.push_back(std::move(*event_handler).release());
    }

    auto graph = std::make_unique<EquivalenceGraph>(std::move(equivalence_builder));
    return { std::move(state_graphs), std::move(graph) };
}

template<tyr::planning::TaskKind Kind>
auto generate_equivalence_graph(TaskSearchContextList<Kind>& contexts, EquivalencePolicyMode policy_mode) -> EquivalenceGraphConstructionResult<Kind>
{
    switch (policy_mode)
    {
        case EquivalencePolicyMode::IDENTITY:
        {
            auto policy = EquivalencePolicy<IdentityEquivalenceTag> {};
            return generate_equivalence_graph(contexts, policy);
        }
        case EquivalencePolicyMode::GI:
        {
            auto policy = EquivalencePolicy<GIEquivalenceTag> {};
            return generate_equivalence_graph(contexts, policy);
        }
    }

    throw std::runtime_error("Unsupported equivalence policy mode.");
}

template auto generate_equivalence_graph<tyr::planning::GroundTag, EquivalencePolicy<IdentityEquivalenceTag>>(
    TaskSearchContextList<tyr::planning::GroundTag>&,
    EquivalencePolicy<IdentityEquivalenceTag>&) -> EquivalenceGraphConstructionResult<tyr::planning::GroundTag>;

template auto generate_equivalence_graph<tyr::planning::LiftedTag, EquivalencePolicy<IdentityEquivalenceTag>>(
    TaskSearchContextList<tyr::planning::LiftedTag>&,
    EquivalencePolicy<IdentityEquivalenceTag>&) -> EquivalenceGraphConstructionResult<tyr::planning::LiftedTag>;

template auto generate_equivalence_graph<tyr::planning::GroundTag, EquivalencePolicy<GIEquivalenceTag>>(TaskSearchContextList<tyr::planning::GroundTag>&,
                                                                                                        EquivalencePolicy<GIEquivalenceTag>&)
    -> EquivalenceGraphConstructionResult<tyr::planning::GroundTag>;

template auto generate_equivalence_graph<tyr::planning::LiftedTag, EquivalencePolicy<GIEquivalenceTag>>(TaskSearchContextList<tyr::planning::LiftedTag>&,
                                                                                                        EquivalencePolicy<GIEquivalenceTag>&)
    -> EquivalenceGraphConstructionResult<tyr::planning::LiftedTag>;

template auto generate_equivalence_graph<tyr::planning::GroundTag>(TaskSearchContextList<tyr::planning::GroundTag>&, EquivalencePolicyMode)
    -> EquivalenceGraphConstructionResult<tyr::planning::GroundTag>;

template auto generate_equivalence_graph<tyr::planning::LiftedTag>(TaskSearchContextList<tyr::planning::LiftedTag>&, EquivalencePolicyMode)
    -> EquivalenceGraphConstructionResult<tyr::planning::LiftedTag>;

}  // namespace runir::datasets
