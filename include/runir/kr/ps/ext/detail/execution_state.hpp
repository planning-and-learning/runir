#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STATE_HPP_

#include "runir/kr/ps/ext/detail/proof_propagation.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"

#include <cstddef>
#include <optional>
#include <stack>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::ps::ext::detail
{

/// Discovery and first predecessors are permanent; Choose cursors never require state rollback.
/// Only active Choose obligations retain their denotations and cursors on the stack.
template<tyr::TaskKind Kind, typename Unsolvability>
class ExecutionState
{
private:
    const ProgramSearchOptions<Kind>& m_options;
    Unsolvability& m_classifier;
    SuccessorExpander<Kind>& m_expander;
    ProgramStateView<Kind> m_initial;
    tyr::planning::ConjunctiveGoalStrategy<Kind> m_goal_strategy;
    bool m_static_goal_satisfied;
    const std::optional<ygg::CountdownWatch>& m_stopwatch;

    ygg::SegmentedVector<SearchNode<Kind>> m_nodes;
    std::vector<Predecessor<Kind>> m_predecessors;
    ProofPropagation<Kind> m_proof { m_nodes, m_predecessors };
    std::size_t m_num_reached = 0;
    std::stack<ChoiceFrame<Kind>, std::vector<ChoiceFrame<Kind>>> m_choices;
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
        m_goal_strategy(*expander.get_task_context()->search_context->task),
        m_static_goal_satisfied(m_goal_strategy.is_static_goal_satisfied(*expander.get_task_context()->search_context->task)),
        m_stopwatch(stopwatch)
    {
    }

    bool out_of_time() const { return m_stopwatch && m_stopwatch->has_finished(); }
    ProgramSearchStatistics& statistics() { return m_statistics; }
    const auto& nodes() const { return m_nodes; }
    auto& choices() { return m_choices; }
    auto& proof() { return m_proof; }
    const auto& predecessors() const { return m_predecessors; }
    SearchNode<Kind>& search_node(ProgramStateView<Kind> state) { return get_or_create_search_node(state, m_nodes); }

    /// Admit a NEW state; return false only when the state limit prevents admission.
    bool discover(ProgramStateView<Kind> state)
    {
        if (m_num_reached >= m_options.max_num_states)
            return false;
        auto& node = search_node(state);
        node.status = SearchStatus::DISCOVERED;
        ++m_num_reached;
        const auto planning_state = state.get_state();
        node.is_goal = m_static_goal_satisfied && m_goal_strategy.is_dynamic_goal_satisfied(m_initial.get_state(), planning_state);
        node.is_unsolvable = !node.is_goal && m_classifier.is_unsolvable(planning_state);
        return true;
    }

    /// Record every admitted transition, including repeated targets; assign path data only on first arrival.
    /// An admission limit leaves the transition unrecorded, but its generation is already counted.
    std::optional<ProgramProofStatus> record_transition(ProgramStateView<Kind> source,
                                                        const ProgramStep<Kind>& step,
                                                        bool non_singleton_choice = false,
                                                        std::optional<std::size_t> choice = std::nullopt)
    {
        const auto depth = search_node(source).choice_depth + ygg::uint_t(non_singleton_choice);
        const auto target = step.get_target();
        auto& node = search_node(target);
        const bool created = node.status == SearchStatus::NEW;
        if (created && !discover(target))
            return ProgramProofStatus::OUT_OF_STATES;
        const auto& transition = step.get_state_transition();
        const auto action = transition ? std::optional(transition->action) : std::nullopt;
        const auto edge = m_predecessors.size();
        m_predecessors.push_back({ source, target, action, step.rule });
        if (created)
        {
            node.parent_state = source;
            node.action = action;
            node.choice_depth = depth;
        }
        m_proof.add_transition(edge, choice);
        return std::nullopt;
    }

    /// Enumerate one state before descending: successor generation is not reentrant.
    /// Greedy mode keeps the first outcome; universal mode records every ordinary outcome and Choose obligation.
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
                limit = std::visit([&](const auto& value) { return record_expansion(state, value); }, expansion);
                return !limit && m_options.universal;
            },
            [&] { return out_of_time(); });
        if (limit)
            return limit;
        if (out_of_time())
            return ProgramProofStatus::OUT_OF_TIME;
        m_proof.seal(state);
        return std::nullopt;
    }

    /// Return the next applied binding, or an empty result when the cursor is exhausted.
    std::optional<ProgramStep<Kind>> next_binding()
    {
        auto& frame = m_choices.top();
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
    std::optional<ProgramProofStatus> record_expansion(ProgramStateView<Kind> state, Choice<Category> choice)
    {
        if (choice.exhausted())
            search_node(state).is_deadend = true;
        const auto obligation = m_proof.add_choice(state);
        m_choices.push({ state, std::move(choice), obligation });
        return std::nullopt;
    }

    std::optional<ProgramProofStatus> record_expansion(ProgramStateView<Kind> source, const ProgramStep<Kind>& step)
    {
        if (step.status == ProgramOutcome::APPLIED || step.status == ProgramOutcome::RESTORED_CALLER)
            return record_transition(source, step);
        search_node(source).is_open = true;
        return std::nullopt;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
