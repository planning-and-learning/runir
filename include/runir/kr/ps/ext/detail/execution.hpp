#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_HPP_

#include "runir/kr/ps/ext/detail/attempt_analysis.hpp"
#include "runir/kr/ps/ext/detail/proof_graph.hpp"
#include "runir/kr/ps/ext/detail/search_space.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <tuple>
#include <type_traits>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
struct ChoiceFrame
{
    ProgramStateView<Kind> state;
    std::variant<Choice<runir::kr::dl::ConceptTag>, Choice<runir::kr::dl::RoleTag>> choice;
};

/// Executes ordinary rules until a choice, a completed attempt, or a resource limit.
template<tyr::TaskKind Kind, typename Unsolvability>
class Execution
{
private:
    static constexpr auto no_entry = std::numeric_limits<std::size_t>::max();

    struct PendingChoice
    {
        ygg::Index<ProgramState<Kind>> state;
        std::variant<Choice<runir::kr::dl::ConceptTag>, Choice<runir::kr::dl::RoleTag>> choice;
    };

    struct Pending
    {
        std::variant<ygg::Index<ProgramState<Kind>>, PendingChoice> work;
        std::size_t next;
    };

    struct Checkpoint
    {
        std::size_t open_head;
        std::size_t open_size;
        std::size_t reached_size;
        std::size_t edge_count;
        bool failed;
    };

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
    std::vector<Pending> m_open;
    std::size_t m_open_head = no_entry;
    std::vector<ygg::Index<ProgramState<Kind>>> m_reached;
    std::vector<std::tuple<ygg::Index<ProgramState<Kind>>, ygg::Index<ProgramState<Kind>>, ygg::uint_t>> m_selected_edges;
    bool m_failed = false;
    ProgramSearchStatistics m_statistics;
    AttemptAnalysis<Kind> m_attempt_analysis;
    std::vector<Checkpoint> m_checkpoints;

public:
    Execution(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program, const ProgramSearchOptions<Kind>& options, Unsolvability& classifier) :
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

    /// Return the next choice when reached, leaving pending siblings available for rollback.
    std::variant<ChoiceFrame<Kind>, ProgramProofStatus> run()
    {
        if (!m_initial)
        {
            const auto initial = m_expander.initial_state(m_initial_node.unpack());
            if (!discover(initial))
                return ProgramProofStatus::OUT_OF_STATES;
            m_initial = initial.get_index();
            get_or_create_search_node(*m_initial, m_nodes).metric = m_initial_node.get_metric();
            push(*m_initial);
        }
        if (out_of_time())
            return ProgramProofStatus::OUT_OF_TIME;

        const auto protected_size = m_checkpoints.empty() ? 0 : m_checkpoints.back().open_size;
        while (m_open_head != no_entry)
        {
            if (out_of_time())
                return ProgramProofStatus::OUT_OF_TIME;

            const auto position = m_open_head;
            const auto pending = m_open[position];
            m_open_head = pending.next;
            // Pending siblings covered by a live checkpoint must survive being popped.
            if (position + 1 == m_open.size() && position >= protected_size)
                m_open.pop_back();

            if (const auto* choice = std::get_if<PendingChoice>(&pending.work))
                return enter_choice(*choice);
            const auto state = state_view(std::get<ygg::Index<ProgramState<Kind>>>(pending.work));
            auto& node = get_or_create_search_node(state.get_index(), m_nodes);
            if (node.is_goal)
            {
                if (!m_options.universal)
                {
                    m_goal = state.get_index();
                    m_statistics.choice_depth = m_attempt_analysis.assess(m_nodes.size(), m_selected_edges, m_statistics).value();
                    return ProgramProofStatus::SUCCESS;
                }
                continue;
            }
            if (node.is_unsolvable)
            {
                node.is_deadend = true;
                m_failed = true;
                continue;
            }

            ++m_statistics.num_expanded;
            auto limit = std::optional<ProgramProofStatus> {};
            m_expander.for_each_successor(
                state,
                tyr::planning::Node<Kind>(state.get_state(), node.metric),
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
                                limit = apply_step(state, value);
                            else
                                push(PendingChoice { state.get_index(), value });
                        },
                        expansion);
                    return !limit && m_options.universal;
                },
                [&] { return out_of_time(); });
            if (limit)
                return *limit;
            if (out_of_time())
                return ProgramProofStatus::OUT_OF_TIME;
        }

        if (out_of_time())
            return ProgramProofStatus::OUT_OF_TIME;
        const auto depth = m_attempt_analysis.assess(m_nodes.size(), m_selected_edges, m_statistics);
        if (m_failed || !depth)
            return ProgramProofStatus::FAILURE;
        m_statistics.choice_depth = *depth;
        return ProgramProofStatus::SUCCESS;
    }

    /// Execute the current binding and continue ordinary execution until the next choice or result.
    std::variant<ChoiceFrame<Kind>, ProgramProofStatus> run(const ChoiceFrame<Kind>& frame)
    {
        if (out_of_time())
            return ProgramProofStatus::OUT_OF_TIME;
        const auto limit = std::visit([&](const auto& choice) { return apply_choice(frame.state, choice); }, frame.choice);
        if (limit)
            return *limit;
        return run();
    }

    /// Undo the failed continuation and advance its binding; exhausted choices release their execution scope.
    bool advance(ChoiceFrame<Kind>& frame)
    {
        return std::visit(
            [&](auto& choice)
            {
                m_statistics.num_backtracks += !choice.exhausted();
                restore(m_checkpoints.back());
                if (!choice.exhausted())
                    choice.advance();
                if (!choice.exhausted())
                    return true;
                m_checkpoints.pop_back();
                return false;
            },
            frame.choice);
    }

    ProgramProofResults<Kind> finish(ProgramProofStatus status)
    {
        if (status != ProgramProofStatus::SUCCESS)
            m_attempt_analysis.assess(m_nodes.size(), m_selected_edges, m_statistics);
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
    ProgramStateView<Kind> state_view(ygg::Index<ProgramState<Kind>> state) const { return { state, *m_task_context->execution_repository }; }

    bool out_of_time() const { return m_stopwatch && m_stopwatch->has_finished(); }

    void restore(const Checkpoint& checkpoint)
    {
        m_open_head = checkpoint.open_head;
        m_open.erase(m_open.begin() + checkpoint.open_size, m_open.end());
        while (m_reached.size() > checkpoint.reached_size)
        {
            auto& node = get_or_create_search_node(m_reached.back(), m_nodes);
            node.parent_state = ygg::Index<ProgramState<Kind>>::max();
            node.planning_successor.reset();
            m_reached.pop_back();
        }
        m_selected_edges.resize(checkpoint.edge_count);
        m_failed = checkpoint.failed;
    }

    std::variant<ChoiceFrame<Kind>, ProgramProofStatus> enter_choice(const PendingChoice& pending)
    {
        if (out_of_time())
            return ProgramProofStatus::OUT_OF_TIME;
        m_statistics.num_choice_points += std::visit([](const auto& choice) { return choice.has_alternatives(); }, pending.choice);
        m_checkpoints.push_back({ m_open_head, m_open.size(), m_reached.size(), m_selected_edges.size(), m_failed });
        return ChoiceFrame<Kind> { state_view(pending.state), pending.choice };
    }

    template<runir::kr::dl::CategoryTag Category>
    std::optional<ProgramProofStatus> apply_choice(ProgramStateView<Kind> source, const Choice<Category>& choice)
    {
        const auto& node = m_nodes[ygg::uint_t(source.get_index())];
        const auto step = m_expander.apply_choice(source, tyr::planning::Node<Kind>(source.get_state(), node.metric), choice, m_statistics);
        if (step.status == ProgramOutcome::APPLIED)
        {
            ++m_statistics.num_binding_attempts;
            return record_transition(source, step, ygg::uint_t(choice.has_alternatives()));
        }
        get_or_create_search_node(source.get_index(), m_nodes).is_deadend = true;
        m_failed = true;
        return std::nullopt;
    }

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

    void push(std::variant<ygg::Index<ProgramState<Kind>>, PendingChoice> work)
    {
        const auto next = m_open_head;
        m_open_head = m_open.size();
        m_open.push_back({ std::move(work), next });
    }

    std::optional<ProgramProofStatus> apply_step(ProgramStateView<Kind> source, const ProgramStep<Kind>& step)
    {
        if (step.status == ProgramOutcome::OUT_OF_TIME)
            return ProgramProofStatus::OUT_OF_TIME;
        if (step.status == ProgramOutcome::OUT_OF_STATES)
            return ProgramProofStatus::OUT_OF_STATES;
        if (step.status == ProgramOutcome::APPLIED || step.status == ProgramOutcome::RESTORED_CALLER)
            return record_transition(source, step, 0);
        get_or_create_search_node(source.get_index(), m_nodes).is_open = true;
        m_failed = true;
        return std::nullopt;
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
        m_selected_edges.emplace_back(source.get_index(), target, weight);
        auto& node = get_or_create_search_node(target, m_nodes);
        // The root has no predecessor; every other scheduled state has one until rollback.
        if (target != *m_initial && node.parent_state == ygg::Index<ProgramState<Kind>>::max())
        {
            node.parent_state = source.get_index();
            node.planning_successor = step.planning_successor;
            node.metric = step.planning_successor ? step.planning_successor->node.get_metric() : source_node.metric;
            node.choice_depth = depth;
            m_reached.push_back(target);
            push(target);
        }
        return std::nullopt;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
