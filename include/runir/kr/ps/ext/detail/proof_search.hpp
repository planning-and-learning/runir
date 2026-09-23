#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/execution_state.hpp"
#include "runir/kr/ps/ext/detail/proof_graph.hpp"
#include "runir/kr/ps/ext/detail/search_space.hpp"
#include "runir/kr/ps/ext/program_executor.hpp"
#include "runir/kr/ps/unsolvability.hpp"

#include <cstddef>
#include <optional>
#include <variant>
#include <vector>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::ps::ext
{
namespace detail
{

template<tyr::TaskKind Kind>
struct SearchFrame
{
    enum class Phase
    {
        ORDINARY,
        CHOOSE,
        BINDING,
    };

    ProgramStateView<Kind> state;
    std::size_t successors_begin;
    std::size_t successor;
    std::size_t choices_begin;
    std::size_t choice;
    bool replay;
    std::size_t binding = 0;
    Phase phase = Phase::ORDINARY;
    SearchStatus result = SearchStatus::SUCCESS;
    SearchStatus choice_result = SearchStatus::FAILURE;

    // Ordinary continuations are conjunctive; one pending child cannot override a definite failure.
    void require(SearchStatus child)
    {
        if (child == SearchStatus::FAILURE || (child == SearchStatus::PENDING && result == SearchStatus::SUCCESS))
            result = child;
    }

    void accept(SearchStatus child)
    {
        if (phase == Phase::ORDINARY)
            require(child);
        else if (child == SearchStatus::SUCCESS)
        {
            --choice;
            phase = Phase::CHOOSE;
        }
        else if (child == SearchStatus::PENDING)
            choice_result = child;
    }
};

/// DFS establishes completion while unwinding. Only pending cyclic dependencies need another walk of recorded edges.
template<tyr::TaskKind Kind, typename Unsolvability>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program, const ProgramSearchOptions<Kind>& options, Unsolvability& classifier)
    -> ProgramProofResults<Kind>
{
    auto expander = SuccessorExpander<Kind>(task_context, program);
    const auto& search_context = *task_context->search_context;
    const auto initial_node = search_context.successor_generator->get_packed_initial_node(*search_context.state_repository, *search_context.axiom_evaluator);
    const auto stopwatch = options.max_time ? std::optional<ygg::CountdownWatch>(*options.max_time) : std::nullopt;
    const auto initial = expander.initial_state(initial_node.unpack());
    auto execution = ExecutionState<Kind, Unsolvability>(expander, initial, options, classifier, stopwatch);
    const auto admitted = execution.discover(initial).has_value();
    auto goal = std::optional<ProgramStateView<Kind>> {};

    const auto finish = [&](ProgramProofStatus status)
    {
        auto result = ProgramProofResults<Kind> {};
        result.task_context_owner = task_context;
        result.status = status;
        result.statistics = execution.statistics();
        build_proof_graph(result, execution.nodes(), execution.predecessors(), admitted ? std::optional(initial) : std::nullopt);
        if (status == ProgramProofStatus::SUCCESS && goal)
        {
            result.statistics.choice_depth = execution.search_node(*goal).choice_depth;
            if (!options.universal)
                result.plan = extract_total_ordered_plan(*goal, execution.nodes(), initial_node, *task_context);
        }
        return result;
    };

    if (!admitted)
        return finish(ProgramProofStatus::OUT_OF_STATES);

    using Phase = typename SearchFrame<Kind>::Phase;
    auto stack = std::vector<SearchFrame<Kind>> {};
    auto next = std::optional(initial);
    auto returned = std::optional<SearchStatus> {};
    while (true)
    {
        if (execution.out_of_time())
            return finish(ProgramProofStatus::OUT_OF_TIME);
        if (next)
        {
            const auto state = *next;
            next.reset();
            auto& node = execution.search_node(state);
            if (node.status == SearchStatus::ACTIVE)
            {
                returned = SearchStatus::PENDING;
                continue;
            }
            const auto replay = node.status == SearchStatus::PENDING;
            const auto successors_begin = replay ? 0 : execution.predecessors().size();
            const auto choices_begin = replay ? 0 : execution.choices().size();
            if (node.status == SearchStatus::NEW)
            {
                if (node.is_goal)
                {
                    if (!goal)
                        goal = state;
                    node.status = SearchStatus::SUCCESS;
                }
                else if (node.is_unsolvable)
                {
                    execution.mark_deadend(state);
                    node.status = SearchStatus::FAILURE;
                }
                else if (const auto limit = execution.expand(state))
                    return finish(*limit);
            }
            if (node.status == SearchStatus::SUCCESS || node.status == SearchStatus::FAILURE)
            {
                returned = node.status;
                continue;
            }
            node.status = SearchStatus::ACTIVE;
            stack.push_back({ state, successors_begin, execution.predecessors().size(), choices_begin, execution.choices().size(), replay });
            if (node.is_open)
                stack.back().result = SearchStatus::FAILURE;
        }
        if (returned)
        {
            if (stack.empty())
                return finish(*returned == SearchStatus::SUCCESS ? ProgramProofStatus::SUCCESS : ProgramProofStatus::FAILURE);
            stack.back().accept(*returned);
            returned.reset();
        }

        auto& frame = stack.back();
        auto& node = execution.search_node(frame.state);
        if (frame.phase == Phase::ORDINARY)
        {
            if (frame.successor != frame.successors_begin)
            {
                const auto& edge = execution.predecessors()[--frame.successor];
                if (edge.source != frame.state)
                    continue;
                if (!edge.rule)
                    next = edge.target;
                else
                    ygg::visit(
                        [&](auto rule)
                        {
                            if constexpr (!ChooseRuleView<decltype(rule)>)
                                next = edge.target;
                        },
                        edge.rule->get_variant());
                continue;
            }
            frame.phase = Phase::CHOOSE;
        }
        if (frame.choice != frame.choices_begin)
        {
            const auto index = frame.choice - 1;
            const auto& choice_frame = execution.choices()[index];
            if (choice_frame.state != frame.state)
            {
                --frame.choice;
                continue;
            }
            if (frame.phase == Phase::CHOOSE)
            {
                frame.choice_result = SearchStatus::FAILURE;
                // ponytail: pending cyclic revisits scan existing records; add an outgoing index only if profiling warrants it.
                frame.binding = frame.replay ? execution.predecessors().size() : 0;
                frame.phase = Phase::BINDING;
            }
            if (frame.binding != 0)
            {
                const auto& edge = execution.predecessors()[--frame.binding];
                if (edge.source == frame.state)
                    std::visit(
                        [&](const auto& choice)
                        {
                            if (edge.rule == choice.rule)
                                next = edge.target;
                        },
                        choice_frame.choice);
                continue;
            }
            if (const auto step = execution.next_binding(index))
            {
                if (step->status == ProgramOutcome::OUT_OF_TIME)
                    return finish(ProgramProofStatus::OUT_OF_TIME);
                if (step->status == ProgramOutcome::OUT_OF_STATES)
                    return finish(ProgramProofStatus::OUT_OF_STATES);
                if (step->status == ProgramOutcome::APPLIED)
                {
                    const auto non_singleton = std::visit([](const auto& choice) { return choice.has_alternatives(); }, choice_frame.choice);
                    if (const auto limit = execution.record_transition(frame.state, *step, non_singleton))
                        return finish(*limit);
                    next = step->get_target();
                    continue;
                }
                execution.mark_deadend(frame.state);
            }
            frame.require(frame.choice_result);
            frame.phase = Phase::CHOOSE;
            --frame.choice;
            continue;
        }
        node.status = frame.result;
        returned = frame.result;
        stack.pop_back();
    }
}

}  // namespace detail

template<tyr::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context_owner,
                   ProgramView program,
                   const ProgramSearchOptions<Kind>& options) -> ProgramProofResults<Kind>
{
    if (options.classifier)
    {
        auto classifier = ClassifierUnsolvability<Kind>(*task_context_owner, *options.classifier);
        return detail::find_solution(task_context_owner, program, options, classifier);
    }
    auto classifier = NoUnsolvability {};
    return detail::find_solution(task_context_owner, program, options, classifier);
}

}  // namespace runir::kr::ps::ext

#endif
