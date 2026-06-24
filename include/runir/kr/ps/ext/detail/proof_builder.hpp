#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_BUILDER_HPP_

#include "runir/graphs/algorithms.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <optional>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <yggdrasil/semantics/equal_to.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::kr::ps::ext::detail
{

template<typename Graph>
class CycleVisitor : public graphs::bgl::TraversalVisitor<Graph>
{
private:
    const Graph& m_graph;
    ygg::UnorderedMap<graphs::VertexIndex, graphs::VertexIndex> m_parent;
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
    ygg::UnorderedMap<ModuleProgramProofVertexLabel<Kind>, graphs::VertexIndex> m_vertex_to_index;
    SuccessorExpander<Kind> m_expander;

public:
    ModuleProgramProofBuilder(const runir::datasets::TaskSearchContext<Kind>& search_context,
                              const tyr::planning::Node<Kind>& initial_node,
                              runir::datasets::TaskSearchContextPtr<Kind> context_owner = {}) :
        m_result(),
        m_builder(),
        m_vertex_to_index(),
        m_expander(search_context, initial_node)
    {
        m_result.context_owner = std::move(context_owner);
    }

    bool is_goal(const tyr::planning::StateView<Kind>& state) { return m_expander.is_goal(state); }

    auto internal_steps(const EvaluationContext<Kind>& context) { return m_expander.internal_steps(context); }

    auto control_steps(const runir::datasets::TaskSearchContext<Kind>& search_context,
                       const EvaluationContext<Kind>& context,
                       const ModuleExecutionOptions<Kind>& options)
    {
        return m_expander.control_steps(search_context, context, options);
    }

    auto get_or_create_vertex(const EvaluationContext<Kind>& context, MemoryStateVariant memory_state, bool is_initial, bool is_alive, bool is_unsolvable)
    {
        auto label = m_expander.make_vertex_label(context, std::move(memory_state), is_initial, is_alive, is_unsolvable);
        if (const auto it = m_vertex_to_index.find(label); it != m_vertex_to_index.end())
            return std::pair(it->second, false);

        const auto vertex = m_builder.add_vertex(label);
        m_vertex_to_index.emplace(std::move(label), vertex);
        return std::pair(vertex, true);
    }

    graphs::EdgeIndex add_edge(graphs::VertexIndex source,
                               graphs::VertexIndex target,
                               std::optional<datasets::StateGraphEdgeLabel> state_transition,
                               std::optional<RuleVariantView> rule = std::nullopt)
    {
        return m_builder.add_directed_edge(source, target, ModuleProgramProofEdgeLabel { std::move(state_transition), rule });
    }

    void set_two_vertex_cycle(graphs::VertexIndex target, graphs::VertexIndex source) { m_result.cycle = graphs::VertexIndexList { target, source, target }; }

    void add_deadend_transition(graphs::EdgeIndex edge) { m_result.deadend_transitions.push_back(edge); }
    void add_open_state(graphs::VertexIndex vertex) { m_result.open_states.push_back(vertex); }
    void set_final_state(tyr::planning::StateView<Kind> state) { m_result.final_state = std::move(state); }
    void set_plan(tyr::planning::Plan<Kind> plan) { m_result.plan = std::move(plan); }

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
