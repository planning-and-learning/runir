#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_

#include "runir/kr/ps/ext/detail/search_node.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"

#include <concepts>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::ps::ext::detail
{

/// Discovery and first predecessors are permanent; Choose cursors never require state rollback.
template<tyr::TaskKind Kind, typename Unsolvability>
class ExecutionState
{
private:
    const ProgramSearchOptions<Kind>& m_options;
    Unsolvability& m_classifier;
    SuccessorExpander<Kind>& m_expander;
    ProgramStateView<Kind> m_initial;
    tyr::planning::StateView<Kind> m_initial_planning_state;
    tyr::planning::ConjunctiveGoalStrategy<Kind> m_goal_strategy;
    bool m_static_goal_satisfied;
    const std::optional<ygg::CountdownWatch>& m_stopwatch;

    ygg::SegmentedVector<SearchNode<Kind>> m_nodes;
    std::vector<Predecessor<Kind>> m_predecessors;
    std::size_t m_num_reached = 0;
    std::vector<ChoiceFrame<Kind>> m_choices;
    ProgramSearchStatistics m_statistics;

public:
    ExecutionState(SuccessorExpander<Kind>& expander,
                   ProgramStateView<Kind> initial,
                   const ProgramSearchOptions<Kind>& options,
                   Unsolvability& classifier,
                   const std::optional<ygg::CountdownWatch>& stopwatch) :
        m_options(options),
        m_classifier(classifier),
        m_expander(expander),
        m_initial(initial),
        m_initial_planning_state(initial.get_state()),
        m_goal_strategy(*expander.get_task_context()->search_context->task),
        m_static_goal_satisfied(m_goal_strategy.is_static_goal_satisfied(*expander.get_task_context()->search_context->task)),
        m_stopwatch(stopwatch)
    {
    }

    bool out_of_time() const { return m_stopwatch && m_stopwatch->has_finished(); }
    ProgramSearchStatistics& statistics() { return m_statistics; }
    const auto& nodes() const { return m_nodes; }
    const auto& choices() const { return m_choices; }
    const auto& predecessors() const { return m_predecessors; }
    SearchNode<Kind>& search_node(ProgramStateView<Kind> state) { return get_or_create_search_node(state, m_nodes); }

    void mark_deadend(ProgramStateView<Kind> state) { search_node(state).is_deadend = true; }

    std::optional<bool> discover(ProgramStateView<Kind> state)
    {
        auto& node = search_node(state);
        if ((m_num_reached != 0 && state == m_initial) || node.parent_state)
            return false;
        if (m_num_reached >= m_options.max_num_states)
            return std::nullopt;
        ++m_num_reached;
        const auto planning_state = state.get_state();
        node.is_goal = m_static_goal_satisfied && m_goal_strategy.is_dynamic_goal_satisfied(m_initial_planning_state, planning_state);
        node.is_unsolvable = !node.is_goal && m_classifier.is_unsolvable(planning_state);
        return true;
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

    /// Enumerate one state before descending: successor generation is not reentrant.
    std::optional<ProgramProofStatus> expand(ProgramStateView<Kind> state)
    {
        ++m_statistics.num_expanded;
        auto limit = std::optional<ProgramProofStatus> {};
        m_expander.for_each_successor(
            state,
            tyr::planning::Node<Kind>(state.get_state(), 0),
            m_statistics,
            [&](const typename SuccessorExpander<Kind>::Expansion& expansion)
            {
                if (out_of_time())
                {
                    limit = ProgramProofStatus::OUT_OF_TIME;
                    return false;
                }
                std::visit(
                    [&](const auto& value)
                    {
                        if constexpr (std::same_as<std::decay_t<decltype(value)>, ProgramStep<Kind>>)
                            limit = record_step(state, value);
                        else
                            add_choice(state, value);
                    },
                    expansion);
                return !limit && m_options.universal;
            },
            [&] { return out_of_time(); });
        if (limit)
            return limit;
        if (out_of_time())
            return ProgramProofStatus::OUT_OF_TIME;
        return std::nullopt;
    }

    /// An empty result means the binding cursor is exhausted; otherwise the step carries its target and outcome.
    std::optional<ProgramStep<Kind>> next_binding(std::size_t index)
    {
        auto& frame = m_choices[index];
        return std::visit(
            [&](auto& choice) -> std::optional<ProgramStep<Kind>>
            {
                if (choice.exhausted())
                    return std::nullopt;
                auto step = m_expander.apply_choice(frame.state, tyr::planning::Node<Kind>(frame.state.get_state(), 0), choice, m_statistics);
                choice.advance();
                return step;
            },
            frame.choice);
    }

private:
    template<runir::kr::dl::CategoryTag Category>
    void add_choice(ProgramStateView<Kind> state, Choice<Category> choice)
    {
        if (choice.exhausted())
            mark_deadend(state);
        m_choices.push_back({ state, std::move(choice) });
    }

    std::optional<ProgramProofStatus> record_step(ProgramStateView<Kind> source, const ProgramStep<Kind>& step)
    {
        if (step.status == ProgramOutcome::OUT_OF_TIME)
            return ProgramProofStatus::OUT_OF_TIME;
        if (step.status == ProgramOutcome::OUT_OF_STATES)
            return ProgramProofStatus::OUT_OF_STATES;
        if (step.status == ProgramOutcome::APPLIED || step.status == ProgramOutcome::RESTORED_CALLER)
            return record_transition(source, step);
        search_node(source).is_open = true;
        return std::nullopt;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
