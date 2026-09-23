#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_

#include "runir/kr/ps/ext/detail/execution_attempt.hpp"
#include "runir/kr/ps/ext/detail/execution_frontier.hpp"
#include "runir/kr/ps/ext/detail/proof_graph.hpp"
#include "runir/kr/ps/ext/detail/search_space.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::ps::ext::detail
{

/// Independent frontier and attempt snapshots. Restore nested choices in LIFO order.
template<tyr::TaskKind Kind>
struct ExecutionCheckpoint
{
    typename ExecutionFrontier<Kind>::Checkpoint frontier;
    typename ExecutionAttempt<Kind>::Checkpoint attempt;
};

/// Shared search data and reversible bookkeeping, independent of binding selection and traversal policy.
template<tyr::TaskKind Kind, typename Unsolvability>
class ExecutionState
{
public:
    using Work = typename ExecutionFrontier<Kind>::Work;

private:
    runir::kr::TaskContextPtr<Kind> m_task_context;
    const ProgramSearchOptions<Kind>& m_options;
    Unsolvability& m_classifier;
    SuccessorExpander<Kind> m_expander;
    tyr::planning::PackedNode<Kind> m_initial_node;
    tyr::planning::StateView<Kind> m_initial_planning_state;
    tyr::planning::ConjunctiveGoalStrategy<Kind> m_goal_strategy;
    bool m_static_goal_satisfied;
    std::optional<ygg::CountdownWatch> m_stopwatch;

    ygg::SegmentedVector<SearchNode<Kind>> m_nodes;
    ygg::UnorderedSet<Predecessor<Kind>> m_predecessors;
    std::size_t m_num_reached = 0;
    std::optional<ygg::Index<ProgramState<Kind>>> m_initial;
    std::optional<ygg::Index<ProgramState<Kind>>> m_goal;
    ExecutionFrontier<Kind> m_frontier;
    ExecutionAttempt<Kind> m_attempt;
    ProgramSearchStatistics m_statistics;

public:
    ExecutionState(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program, const ProgramSearchOptions<Kind>& options, Unsolvability& classifier) :
        m_task_context(std::move(task_context)),
        m_options(options),
        m_classifier(classifier),
        m_expander(m_task_context, program),
        m_initial_node(m_task_context->search_context->successor_generator->get_packed_initial_node(*m_task_context->search_context->state_repository,
                                                                                                    *m_task_context->search_context->axiom_evaluator)),
        m_initial_planning_state(m_initial_node.get_state().unpack()),
        m_goal_strategy(*m_task_context->search_context->task),
        m_static_goal_satisfied(m_goal_strategy.is_static_goal_satisfied(*m_task_context->search_context->task)),
        m_stopwatch(options.max_time ? std::optional(ygg::CountdownWatch(*options.max_time)) : std::nullopt)
    {
    }

    bool initialize()
    {
        if (!m_initial)
        {
            const auto initial = m_expander.initial_state(m_initial_node.unpack());
            if (!discover(initial))
                return false;
            m_initial = initial.get_index();
            search_node(*m_initial).metric = m_initial_node.get_metric();
            push(*m_initial);
        }
        return true;
    }

    bool out_of_time() const { return m_stopwatch && m_stopwatch->has_finished(); }
    bool universal() const { return m_options.universal; }
    SuccessorExpander<Kind>& expander() { return m_expander; }
    ProgramSearchStatistics& statistics() { return m_statistics; }
    ProgramStateView<Kind> state_view(ygg::Index<ProgramState<Kind>> state) const { return { state, *m_task_context->execution_repository }; }
    SearchNode<Kind>& search_node(ygg::Index<ProgramState<Kind>> state) { return get_or_create_search_node(state, m_nodes); }

    bool has_pending() const { return !m_frontier.empty(); }
    void push(Work work) { m_frontier.push(std::move(work)); }
    Work pop() { return m_frontier.pop(); }

    ExecutionCheckpoint<Kind> checkpoint() const { return { m_frontier.checkpoint(), m_attempt.checkpoint() }; }

    void restore(const ExecutionCheckpoint<Kind>& checkpoint)
    {
        m_frontier.restore(checkpoint.frontier);
        m_attempt.restore(checkpoint.attempt, m_nodes);
    }

    void mark_deadend(ygg::Index<ProgramState<Kind>> state)
    {
        search_node(state).is_deadend = true;
        m_attempt.mark_failed();
    }

    void mark_open(ygg::Index<ProgramState<Kind>> state)
    {
        search_node(state).is_open = true;
        m_attempt.mark_failed();
    }

    std::optional<ProgramProofStatus> record_transition(ProgramStateView<Kind> source, const ProgramStep<Kind>& step, ygg::uint_t weight)
    {
        const auto& source_node = m_nodes[ygg::uint_t(source.get_index())];
        const auto depth = source_node.choice_depth + weight;
        m_statistics.max_choice_depth = std::max(m_statistics.max_choice_depth, depth);
        const auto target = step.get_target().get_index();
        if (!discover(step.get_target()))
            return ProgramProofStatus::OUT_OF_STATES;
        const auto& transition = step.get_state_transition();
        m_predecessors.emplace(source.get_index(),
                               target,
                               transition ? std::optional(transition->action) : std::nullopt,
                               transition ? transition->cost : ygg::float_t(0),
                               step.rule,
                               m_predecessors.size());
        m_attempt.record_transition(source.get_index(), target, weight);
        auto& node = get_or_create_search_node(target, m_nodes);
        // The root has no predecessor; every other scheduled state has one until rollback.
        if (target != *m_initial && node.parent_state == ygg::Index<ProgramState<Kind>>::max())
        {
            node.parent_state = source.get_index();
            node.planning_successor = step.planning_successor;
            node.metric = step.planning_successor ? step.planning_successor->node.get_metric() : source_node.metric;
            node.choice_depth = depth;
            m_attempt.record_reached(target);
            push(target);
        }
        return std::nullopt;
    }

    void select_goal(ygg::Index<ProgramState<Kind>> state)
    {
        m_goal = state;
        m_statistics.choice_depth = m_attempt.assess(m_nodes.size(), m_statistics).value();
    }

    ProgramProofStatus assess_attempt()
    {
        const auto depth = m_attempt.assess(m_nodes.size(), m_statistics);
        if (m_attempt.has_failed() || !depth)
            return ProgramProofStatus::FAILURE;
        m_statistics.choice_depth = *depth;
        return ProgramProofStatus::SUCCESS;
    }

    ProgramProofResults<Kind> finish(ProgramProofStatus status)
    {
        if (status != ProgramProofStatus::SUCCESS)
            m_attempt.assess(m_nodes.size(), m_statistics);
        auto result = ProgramProofResults<Kind> {};
        result.task_context_owner = m_task_context;
        result.status = status;
        build_proof_graph(result, m_nodes, m_predecessors, m_initial);
        if (status == ProgramProofStatus::SUCCESS && m_goal)
            result.plan = extract_total_ordered_plan(*m_goal, m_nodes, m_initial_node);
        result.statistics = m_statistics;
        return result;
    }

private:
    bool discover(ProgramStateView<Kind> state)
    {
        auto& node = get_or_create_search_node(state.get_index(), m_nodes);
        if (node.step != SearchNode<Kind>::unreached)
            return true;
        if (m_num_reached >= m_options.max_num_states)
            return false;
        node.step = m_num_reached++;
        const auto planning_state = state.get_state();
        node.is_goal = m_static_goal_satisfied && m_goal_strategy.is_dynamic_goal_satisfied(m_initial_planning_state, planning_state);
        node.is_unsolvable = !node.is_goal && m_classifier.is_unsolvable(planning_state);
        return true;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
