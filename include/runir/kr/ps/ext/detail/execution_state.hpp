#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_

#include "runir/kr/ps/ext/detail/proof_graph.hpp"
#include "runir/kr/ps/ext/detail/search_space.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"

#include <cstddef>
#include <optional>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::ps::ext::detail
{

/// Discovery and first predecessors are permanent; Choose cursors never require state rollback.
template<tyr::TaskKind Kind, typename Unsolvability>
class ExecutionState
{
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
    std::vector<Predecessor<Kind>> m_predecessors;
    std::size_t m_num_reached = 0;
    std::optional<ProgramStateView<Kind>> m_initial;
    std::optional<ProgramStateView<Kind>> m_goal;
    std::vector<ChoiceFrame<Kind>> m_choices;
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
        const auto initial = m_expander.initial_state(m_initial_node.unpack());
        if (!discover(initial))
            return false;
        m_initial = initial;
        return true;
    }

    bool out_of_time() const { return m_stopwatch && m_stopwatch->has_finished(); }
    bool universal() const { return m_options.universal; }
    SuccessorExpander<Kind>& expander() { return m_expander; }
    ProgramSearchStatistics& statistics() { return m_statistics; }
    auto& choices() { return m_choices; }
    const auto& predecessors() const { return m_predecessors; }
    auto initial() const { return *m_initial; }
    SearchNode<Kind>& search_node(ProgramStateView<Kind> state) { return get_or_create_search_node(state, m_nodes); }

    void mark_deadend(ProgramStateView<Kind> state) { search_node(state).is_deadend = true; }
    void mark_open(ProgramStateView<Kind> state) { search_node(state).is_open = true; }

    template<runir::kr::dl::CategoryTag Category>
    void add_choice(ProgramStateView<Kind> state, Choice<Category> choice)
    {
        if (choice.exhausted())
            mark_deadend(state);
        m_choices.push_back({ state, std::move(choice) });
    }

    std::optional<ProgramProofStatus> record_transition(ProgramStateView<Kind> source, const ProgramStep<Kind>& step, bool non_singleton_choice = false)
    {
        const auto depth = search_node(source).choice_depth + ygg::uint_t(non_singleton_choice);
        const auto created = discover(step.get_target());
        if (!created)
            return ProgramProofStatus::OUT_OF_STATES;
        const auto target = step.get_target();
        const auto& transition = step.get_state_transition();
        const auto action = transition ? std::optional(transition->action) : std::nullopt;
        m_predecessors.push_back({ source, target, action, step.rule });
        if (*created)
        {
            auto& node = search_node(target);
            node.parent_state = source;
            node.action = action;
            node.choice_depth = depth;
        }
        return std::nullopt;
    }

    void select_goal(ProgramStateView<Kind> state)
    {
        if (!m_goal)
            m_goal = state;
    }

    ProgramProofResults<Kind> finish(ProgramProofStatus status)
    {
        auto result = ProgramProofResults<Kind> {};
        result.task_context_owner = m_task_context;
        result.status = status;
        build_proof_graph(result, m_nodes, m_predecessors, m_initial);
        if (status == ProgramProofStatus::SUCCESS && m_goal)
        {
            m_statistics.choice_depth = search_node(*m_goal).choice_depth;
            if (!universal())
                result.plan = extract_total_ordered_plan(*m_goal, m_nodes, m_initial_node, *m_task_context);
        }
        result.statistics = m_statistics;
        return result;
    }

private:
    std::optional<bool> discover(ProgramStateView<Kind> state)
    {
        auto& node = search_node(state);
        if (m_initial == state || node.parent_state)
            return false;
        if (m_num_reached >= m_options.max_num_states)
            return std::nullopt;
        ++m_num_reached;
        const auto planning_state = state.get_state();
        node.is_goal = m_static_goal_satisfied && m_goal_strategy.is_dynamic_goal_satisfied(m_initial_planning_state, planning_state);
        node.is_unsolvable = !node.is_goal && m_classifier.is_unsolvable(planning_state);
        return true;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
