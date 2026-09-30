#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/execution_state.hpp"
#include "runir/kr/ps/ext/detail/proof_graph.hpp"
#include "runir/kr/ps/ext/detail/search_space.hpp"
#include "runir/kr/ps/ext/dl/structural_termination.hpp"
#include "runir/kr/ps/ext/program_executor.hpp"
#include "runir/kr/ps/unsolvability.hpp"

#include <cassert>
#include <cstddef>
#include <optional>
#include <stack>
#include <variant>
#include <vector>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::ps::ext
{
namespace detail
{

/// One expansion appends a contiguous run of ordinary diagnostic edges before DFS descends.
/// Consume that run in reverse order, preserving the existing LIFO traversal without an outgoing index.
template<tyr::TaskKind Kind>
struct SearchFrame
{
    ProgramStateView<Kind> state;
    EdgeId begin;
    EdgeId next;
    std::optional<ProgramStateView<Kind>> child = std::nullopt;
    bool choice_child = false;
    bool succeeded = true;
};

/// Evaluate the terminating execution graph in postorder, reusing completed results.
/// Ordinary successors are AND requirements; bindings of each Choose are alternatives.
/// Resource limits retain the explored graph and counters without deciding unfinished states.
template<tyr::TaskKind Kind, typename Unsolvability>
ProgramProofStatus
depth_first_search(ExecutionState<Kind, Unsolvability>& execution, ProgramStateView<Kind> initial, std::optional<ProgramStateView<Kind>>& goal)
{
    auto stack = std::stack<SearchFrame<Kind>, std::vector<SearchFrame<Kind>>> {};
    auto next = std::optional(initial);
    while (true)
    {
        if (execution.out_of_time())
            return ProgramProofStatus::OUT_OF_TIME;

        if (next)
        {
            const auto state = *next;
            next.reset();
            auto& node = execution.search_node(state);
            assert(node.status != SearchStatus::ACTIVE && "Structurally terminating execution cannot have a back edge.");
            if (node.status != SearchStatus::DISCOVERED)
                continue;
            if (node.is_goal)
            {
                if (!goal)
                    goal = state;
                node.status = SearchStatus::SUCCESS;
                continue;
            }
            if (node.is_unsolvable)
            {
                node.is_deadend = true;
                node.status = SearchStatus::FAILURE;
                continue;
            }
            node.status = SearchStatus::ACTIVE;
            const auto begin = execution.predecessors().end_id();
            if (const auto limit = execution.expand(state))
                return *limit;
            // Snapshot ordinary edges before any lazy bindings are recorded for this state.
            stack.push({ state, begin, execution.predecessors().end_id(), std::nullopt, false, !node.is_open && !node.is_deadend });
            continue;
        }

        if (stack.empty())
            return execution.search_node(initial).status == SearchStatus::SUCCESS ? ProgramProofStatus::SUCCESS : ProgramProofStatus::FAILURE;

        auto& frame = stack.top();
        auto& choices = execution.choices();
        if (frame.child)
        {
            const auto status = execution.search_node(*frame.child).status;
            assert(status == SearchStatus::SUCCESS || status == SearchStatus::FAILURE);
            if (frame.choice_child)
            {
                if (status == SearchStatus::SUCCESS)
                    choices.pop();
            }
            else
                frame.succeeded &= status == SearchStatus::SUCCESS;
            frame.child.reset();
        }
        if (frame.next != frame.begin)
        {
            frame.next = Predecessors<Kind>::previous(frame.next);
            frame.child = execution.predecessors()[frame.next].target;
            frame.choice_child = false;
            next = frame.child;
            continue;
        }

        if (!choices.empty() && choices.top().state == frame.state)
        {
            const auto& choice = choices.top();
            if (const auto step = execution.next_binding())
            {
                const auto non_singleton = std::visit([](const auto& binding) { return binding.has_alternatives(); }, choice.choice);
                if (const auto limit = execution.record_transition(frame.state, *step, non_singleton))
                    return *limit;
                frame.child = step->get_target();
                frame.choice_child = true;
                next = frame.child;
                continue;
            }
            frame.succeeded = false;
            choices.pop();
            continue;
        }
        auto& node = execution.search_node(frame.state);
        node.status = frame.succeeded ? SearchStatus::SUCCESS : SearchStatus::FAILURE;
        stack.pop();
    }
}

/// Own the search lifetime, then construct its graph and optional execution plan.
template<tyr::TaskKind Kind, typename Unsolvability>
auto find_solution(SuccessorExpander<Kind>& expander, const ProgramSearchOptions<Kind>& options, Unsolvability& classifier) -> ProgramProofResults<Kind>
{
    const auto& task_context = expander.get_task_context();
    const auto& search_context = *task_context->search_context;
    const auto initial_node = search_context.successor_generator->get_packed_initial_node(*search_context.state_repository, *search_context.axiom_evaluator);
    const auto stopwatch = options.max_time ? std::optional<ygg::CountdownWatch>(*options.max_time) : std::nullopt;
    const auto initial = expander.initial_state(initial_node.get_state().unpack());
    auto execution = ExecutionState<Kind, Unsolvability>(expander, initial, options, classifier, stopwatch);
    const auto admitted = execution.discover(initial);
    auto goal = std::optional<ProgramStateView<Kind>> {};

    const auto status = admitted ? depth_first_search(execution, initial, goal) : ProgramProofStatus::OUT_OF_STATES;

    // Build the diagnostic graph from every explored transition, including rejected choices.
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
}

}  // namespace detail

template<tyr::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context_owner,
                   ProgramView program,
                   const ProgramSearchOptions<Kind>& options) -> ProgramProofResults<Kind>
{
    if (!dl::structural_termination(program).is_terminating())
        throw std::invalid_argument("Ext find_solution requires a structurally terminating program.");

    // Validate the task and program before constructing a classifier that borrows the task context.
    auto expander = SuccessorExpander<Kind>(task_context_owner, program);
    if (options.classifier)
    {
        auto classifier = ClassifierUnsolvability<Kind>(*task_context_owner, *options.classifier);
        return detail::find_solution(expander, options, classifier);
    }
    auto classifier = NoUnsolvability {};
    return detail::find_solution(expander, options, classifier);
}

}  // namespace runir::kr::ps::ext

#endif
