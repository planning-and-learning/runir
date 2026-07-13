#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_BUILDER_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <optional>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/semantics/equal_to.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
class ModuleProgramProofBuilder
{
private:
    ModuleProgramProofResults<Kind> m_result;
    ModuleProgramProofGraphBuilder<Kind> m_builder;
    ygg::UnorderedMap<ModuleProgramProofVertexLabel<Kind>, graphs::VertexIndex> m_vertex_to_index;
    SuccessorExpander<Kind> m_expander;

public:
    ModuleProgramProofBuilder(runir::kr::TaskContext<Kind>& task_context,
                              const tyr::planning::Node<Kind>& initial_node,
                              ModuleProgramView program,
                              runir::kr::TaskContextPtr<Kind> task_context_owner = {}) :
        m_result(),
        m_builder(),
        m_vertex_to_index(),
        m_expander(task_context, initial_node, program)
    {
        m_result.task_context_owner = std::move(task_context_owner);
    }

    bool is_goal(const tyr::planning::StateView<Kind>& state) { return m_expander.is_goal(state); }

    auto load_steps(const EvaluationContext<Kind>& context, auto&& stop) { return m_expander.load_steps_until(context, stop); }
    auto labeled_successors(const EvaluationContext<Kind>& context) { return m_expander.labeled_successors(context); }

    auto control_steps(const EvaluationContext<Kind>& context, const std::vector<tyr::planning::LabeledNode<Kind>>& successors, auto&& stop)
    {
        return m_expander.control_steps_until(context, successors, stop);
    }

    auto get_or_create_vertex(const EvaluationContext<Kind>& context,
                              MemoryStateVariant memory_state,
                              bool is_initial,
                              bool is_alive,
                              bool is_unsolvable,
                              ygg::uint_t max_num_vertices) -> std::optional<std::pair<graphs::VertexIndex, bool>>
    {
        auto label = m_expander.make_vertex_label(context, std::move(memory_state), is_initial, is_alive, is_unsolvable);
        if (const auto it = m_vertex_to_index.find(label); it != m_vertex_to_index.end())
            return std::pair(it->second, false);
        if (m_vertex_to_index.size() >= max_num_vertices)
            return std::nullopt;

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

    void add_deadend_transition(graphs::EdgeIndex edge) { m_result.deadend_transitions.push_back(edge); }
    void add_open_state(graphs::VertexIndex vertex) { m_result.open_states.push_back(vertex); }
    void set_final_state(tyr::planning::StateView<Kind> state) { m_result.final_state = std::move(state); }
    void set_plan(tyr::planning::Plan<Kind> plan) { m_result.plan = std::move(plan); }

    auto finish(ModuleProgramProofStatus status) -> ModuleProgramProofResults<Kind>
    {
        m_result.status = status;
        auto graph = std::make_shared<ModuleProgramProofGraph<Kind>>(std::move(m_builder));
        if (m_result.cycle.empty())
            m_result.cycle = graphs::find_cycle(*graph);
        if (m_result.status == ModuleProgramProofStatus::SUCCESS && !m_result.cycle.empty())
            m_result.status = ModuleProgramProofStatus::FAILURE;
        m_result.graph = std::move(graph);
        return std::move(m_result);
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
