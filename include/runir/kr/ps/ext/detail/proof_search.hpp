#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/execution_state.hpp"
#include "runir/kr/ps/ext/detail/proof_graph.hpp"
#include "runir/kr/ps/ext/detail/search_space.hpp"
#include "runir/kr/ps/ext/program_executor.hpp"
#include "runir/kr/ps/unsolvability.hpp"

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
    std::size_t begin;
    std::size_t next;
};

/// DFS schedules generation; ProofPropagation resolves the AND/OR dependencies independently.
/// Expand a state once, visit ordinary edges, then try each Choose until proved or exhausted.
/// Reaching a previously expanded state adds a dependency but never replays its outgoing edges.
/// An unresolved binding must not block the next alternative: a later exit can prove the cycle.
/// Only after all scheduled work and success notifications drain is an unresolved root a failure.
/// Resource limits instead return their limit status, retaining the explored graph and counters.
template<tyr::TaskKind Kind, typename Unsolvability>
ProgramProofStatus
depth_first_search(ExecutionState<Kind, Unsolvability>& execution, ProgramStateView<Kind> initial, std::optional<ProgramStateView<Kind>>& goal)
{
    auto stack = std::stack<SearchFrame<Kind>, std::vector<SearchFrame<Kind>>> {};
    auto next = std::optional(initial);
    while (true)
    {
        if (execution.out_of_time() || !execution.proof().propagate([&] { return execution.out_of_time(); }))
            return ProgramProofStatus::OUT_OF_TIME;

        if (next)
        {
            const auto state = *next;
            next.reset();
            auto& node = execution.search_node(state);
            if (node.status != SearchStatus::DISCOVERED)
                continue;
            if (node.is_goal)
            {
                if (!goal)
                    goal = state;
                execution.proof().succeed(state);
                continue;
            }
            if (node.is_unsolvable)
            {
                node.is_deadend = true;
                node.status = SearchStatus::FAILURE;
                continue;
            }
            node.status = SearchStatus::ACTIVE;
            const auto begin = execution.predecessors().size();
            if (const auto limit = execution.expand(state))
                return *limit;
            // Snapshot ordinary edges before any lazy bindings are recorded for this state.
            stack.push({ state, begin, execution.predecessors().size() });
            continue;
        }

        // Propagation above also runs after the final frame/goal, so no success event is lost.
        if (stack.empty())
            return execution.search_node(initial).status == SearchStatus::SUCCESS ? ProgramProofStatus::SUCCESS : ProgramProofStatus::FAILURE;

        auto& frame = stack.top();
        if (frame.next != frame.begin)
        {
            next = execution.predecessors()[--frame.next].target;
            continue;
        }

        auto& choices = execution.choices();
        if (!choices.empty() && choices.top().state == frame.state)
        {
            const auto& choice = choices.top();
            if (!execution.proof().choice_succeeded(choice.obligation))
            {
                if (const auto step = execution.next_binding())
                {
                    const auto non_singleton = std::visit([](const auto& binding) { return binding.has_alternatives(); }, choice.choice);
                    if (const auto limit = execution.record_transition(frame.state, *step, non_singleton, choice.obligation))
                        return *limit;
                    next = step->get_target();
                    continue;
                }
            }
            choices.pop();
            continue;
        }
        auto& node = execution.search_node(frame.state);
        if (node.status == SearchStatus::ACTIVE)
            node.status = SearchStatus::PENDING;
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
    const auto initial = expander.initial_state(initial_node.unpack());
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
