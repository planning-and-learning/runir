#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_BUILDER_HPP_

#include "runir/graphs/algorithms.hpp"
#include "runir/kr/ps/ext/detail/configuration.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <optional>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <unordered_map>
#include <utility>

namespace runir::kr::ps::ext::detail
{

template<typename Graph>
class CycleVisitor : public graphs::bgl::TraversalVisitor<Graph>
{
private:
    const Graph& m_graph;
    tyr::UnorderedMap<graphs::VertexIndex, graphs::VertexIndex> m_parent;
    graphs::VertexIndexList m_cycle;

public:
    explicit CycleVisitor(const Graph& graph) : m_graph(graph) {}

    void tree_edge(graphs::EdgeIndex edge) override { m_parent[m_graph.get_target(edge)] = m_graph.get_source(edge); }

    void back_edge(graphs::EdgeIndex edge) override
    {
        if (!m_cycle.empty())
            return;

        const auto source = m_graph.get_source(edge);
        const auto target = m_graph.get_target(edge);

        m_cycle.push_back(target);
        for (auto vertex = source; vertex != target; vertex = m_parent.at(vertex))
            m_cycle.push_back(vertex);
        m_cycle.push_back(target);
        std::ranges::reverse(m_cycle);
    }

    const auto& get_cycle() const noexcept { return m_cycle; }
};

template<typename Graph>
auto find_cycle(const Graph& graph) -> graphs::VertexIndexList
{
    auto visitor = CycleVisitor<Graph>(graph);
    auto sources = graphs::VertexIndexList {};
    sources.reserve(graph.get_num_vertices());
    for (auto vertex : graph.get_vertex_indices())
        sources.push_back(vertex);

    graphs::algorithms::depth_first_visit(graph, sources, visitor);
    return visitor.get_cycle();
}

template<tyr::planning::TaskKind Kind>
class ModuleProgramProofBuilder
{
private:
    ModuleProgramProofResults<Kind> m_result;
    ModuleProgramProofGraphBuilder<Kind> m_builder;
    std::unordered_map<std::string, graphs::VertexIndex> m_configuration_to_vertex;
    tyr::planning::ConjunctiveGoalStrategy<Kind> m_task_goal_strategy;
    tyr::planning::StateView<Kind> m_initial_state;
    bool m_static_goal_satisfied;

    ModuleProgramProofVertexLabel<Kind> make_vertex_label(const EvaluationContext<Kind>& context, bool is_initial, bool is_alive, bool is_unsolvable)
    {
        auto label = ModuleProgramProofVertexLabel<Kind> { context.get_state(),
                                                           context.get_module(),
                                                           context.get_memory_state(),
                                                           context.get_repository_owner(),
                                                           context.get_call_stack().size(),
                                                           is_goal(context.get_state()) ? tyr::float_t(0) : std::numeric_limits<tyr::float_t>::infinity() };

        label.is_initial = is_initial;
        label.is_goal = is_goal(context.get_state());
        label.is_alive = is_alive;
        label.is_unsolvable = is_unsolvable;
        return label;
    }

public:
    ModuleProgramProofBuilder(const runir::datasets::TaskSearchContext<Kind>& search_context,
                              const tyr::planning::Node<Kind>& initial_node,
                              ConstRepositoryPtr repository_owner = {}) :
        m_result(),
        m_builder(),
        m_configuration_to_vertex(),
        m_task_goal_strategy(*search_context.task),
        m_initial_state(initial_node.get_state()),
        m_static_goal_satisfied(m_task_goal_strategy.is_static_goal_satisfied(*search_context.task))
    {
        m_result.context_owner = std::shared_ptr<const runir::datasets::TaskSearchContext<Kind>>(&search_context, [](const runir::datasets::TaskSearchContext<Kind>*) {});
        m_result.repository_owner = std::move(repository_owner);
    }

    bool is_goal(const tyr::planning::StateView<Kind>& state)
    {
        return m_static_goal_satisfied && m_task_goal_strategy.is_dynamic_goal_satisfied(m_initial_state, state);
    }

    auto get_or_create_vertex(const EvaluationContext<Kind>& context, bool is_initial, bool is_alive, bool is_unsolvable)
    {
        auto label = make_vertex_label(context, is_initial, is_alive, is_unsolvable);
        const auto key = module_program_configuration_key(context);
        if (const auto it = m_configuration_to_vertex.find(key); it != m_configuration_to_vertex.end())
            return std::pair(it->second, false);

        const auto vertex = m_builder.add_vertex(std::move(label));
        m_configuration_to_vertex.emplace(key, vertex);
        return std::pair(vertex, true);
    }

    graphs::EdgeIndex add_edge(graphs::VertexIndex source,
                               graphs::VertexIndex target,
                               std::optional<tyr::Index<tyr::formalism::planning::GroundAction>> action,
                               tyr::float_t cost)
    {
        return m_builder.add_directed_edge(source, target, ModuleProgramProofEdgeLabel { action, cost });
    }

    void set_two_vertex_cycle(graphs::VertexIndex target, graphs::VertexIndex source) { m_result.cycle = graphs::VertexIndexList { target, source, target }; }

    void add_deadend_transition(graphs::EdgeIndex edge) { m_result.deadend_transitions.push_back(edge); }
    void add_open_state(graphs::VertexIndex vertex) { m_result.open_states.push_back(vertex); }

    auto finish(ModuleProgramProofStatus status) -> ModuleProgramProofResults<Kind>
    {
        m_result.status = status;
        auto graph = std::make_shared<ModuleProgramProofGraph<Kind>>(std::move(m_builder));
        if (m_result.cycle.empty())
            m_result.cycle = find_cycle(*graph);
        if (m_result.status == ModuleProgramProofStatus::SUCCESS && !m_result.cycle.empty())
            m_result.status = ModuleProgramProofStatus::FAILURE;
        m_result.graph = std::move(graph);
        return std::move(m_result);
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
