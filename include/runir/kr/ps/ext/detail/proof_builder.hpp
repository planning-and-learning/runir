#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_BUILDER_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_BUILDER_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/dl/semantics/uns/evaluation_context.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"
#include "runir/kr/task_context.hpp"
#include "runir/kr/uns/classify.hpp"

#include <memory>
#include <optional>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
class ModuleProgramProofBuilder
{
private:
    ModuleProgramProofResults<Kind> m_result;
    ModuleProgramProofGraphBuilder<Kind> m_builder;
    ygg::UnorderedMap<ygg::Index<ExecutionState<Kind>>, graphs::VertexIndex> m_vertex_to_index;
    SuccessorExpander<Kind> m_expander;
    std::optional<runir::kr::uns::ClassifierView> m_classifier;

public:
    ModuleProgramProofBuilder(runir::kr::TaskContextPtr<Kind> task_context,
                              ModuleProgramView program,
                              std::optional<runir::kr::uns::ClassifierView> classifier) :
        m_result(),
        m_builder(),
        m_vertex_to_index(),
        m_expander(std::move(task_context), program),
        m_classifier(std::move(classifier))
    {
        m_result.task_context_owner = m_expander.get_task_context();
    }

    auto initial_state() { return m_expander.initial_state(); }

    bool is_goal(const tyr::planning::StateView<Kind>& state) { return m_expander.is_goal(state); }

    bool is_unsolvable(graphs::VertexIndex vertex) const { return m_builder.get_vertex(vertex).get_property().is_unsolvable; }

    void labeled_successors(ExecutionStateView<Kind> state, std::vector<tyr::planning::LabeledNode<Kind>>& out_successors)
    {
        m_expander.labeled_successors(std::move(state), out_successors);
    }

    void steps(ExecutionStateView<Kind> state,
               const std::vector<tyr::planning::LabeledNode<Kind>>& successors,
               auto&& stop,
               std::vector<ModuleProgramStep<Kind>>& out_steps)
    {
        m_expander.steps_until(std::move(state), successors, std::forward<decltype(stop)>(stop), out_steps);
    }

    auto get_or_create_vertex(ExecutionStateView<Kind> state, bool is_initial, bool is_alive, bool is_unsolvable, ygg::uint_t max_num_vertices)
        -> std::optional<std::pair<graphs::VertexIndex, bool>>
    {
        if (const auto it = m_vertex_to_index.find(state.get_index()); it != m_vertex_to_index.end())
            return std::pair(it->second, false);
        if (m_vertex_to_index.size() >= max_num_vertices)
            return std::nullopt;

        const auto goal = is_goal(state.get_state());
        if (!goal && m_classifier)
        {
            auto& task_context = *m_expander.get_task_context();
            auto context = runir::kr::dl::semantics::EvaluationContext<runir::kr::UnsFamilyTag, Kind>(state.get_state(),
                                                                                                      task_context.dl_builder,
                                                                                                      *task_context.dl_denotation_repository);
            is_unsolvable |= runir::kr::uns::classify(*m_classifier, context);
            is_alive &= !is_unsolvable;
        }
        auto label = ModuleProgramProofVertexLabel<Kind> { state, is_initial, goal, is_alive, is_unsolvable };
        const auto vertex = m_builder.add_vertex(label);
        m_vertex_to_index.emplace(state.get_index(), vertex);
        return std::pair(vertex, true);
    }

    void add_edge(graphs::VertexIndex source,
                  graphs::VertexIndex target,
                  std::optional<datasets::StateGraphEdgeLabel> state_transition,
                  std::optional<RuleVariantView> rule = std::nullopt)
    {
        auto label = ModuleProgramProofEdgeLabel {};
        if (state_transition)
            label.state_transition = ModuleProgramProofStateTransition { state_transition->action, state_transition->cost };
        if (rule)
            label.rule = *rule;
        m_builder.add_directed_edge(source, target, std::move(label));
    }

    void add_deadend_state(graphs::VertexIndex vertex) { m_result.deadend_states.push_back(vertex); }
    void add_open_state(graphs::VertexIndex vertex) { m_result.open_states.push_back(vertex); }
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
