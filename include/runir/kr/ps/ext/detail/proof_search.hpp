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

/// A suspended state (AND) or one of its Choose rules (OR), with one accumulated child result.
template<tyr::TaskKind Kind>
struct SearchFrame
{
    ProgramStateView<Kind> state;
    std::size_t edge;
    // State frames combine required continuations with AND; Choose frames combine bindings with OR.
    std::optional<RuleVariantView> rule = std::nullopt;
    SearchStatus result = SearchStatus::SUCCESS;

    /// A conclusive failure dominates AND; a conclusive success dominates OR. PENDING preserves cyclic dependencies.
    void accept(SearchStatus child)
    {
        const auto decisive = rule ? SearchStatus::SUCCESS : SearchStatus::FAILURE;
        if (child == decisive || (child == SearchStatus::PENDING && result != decisive))
            result = child;
    }
};

/// Resolve ordinary continuations conjunctively and each Choose rule existentially.
/// Expand each program state once; revisit only recorded dependencies that remain PENDING.
/// Report the first visited goal for plan reconstruction and choice-depth statistics.
// ponytail: dense cycles can force repeated walks of simple paths; use SCC resolution if cyclic replay becomes a bottleneck.
template<tyr::TaskKind Kind, typename Unsolvability>
ProgramProofStatus
depth_first_search(ExecutionState<Kind, Unsolvability>& execution, ProgramStateView<Kind> initial, std::optional<ProgramStateView<Kind>>& goal)
{
    constexpr auto no_edge = ExecutionState<Kind, Unsolvability>::no_edge;
    auto stack = std::stack<SearchFrame<Kind>, std::vector<SearchFrame<Kind>>> {};
    auto next = std::optional(initial);
    auto returned = std::optional<SearchStatus> {};
    while (true)
    {
        if (execution.out_of_time())
            return ProgramProofStatus::OUT_OF_TIME;

        // Enter a child: cached results return immediately; an active ancestor is an unresolved dependency.
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
            if (node.status == SearchStatus::DISCOVERED)
            {
                if (node.is_goal)
                {
                    if (!goal)
                        goal = state;
                    node.status = SearchStatus::SUCCESS;
                }
                else if (node.is_unsolvable)
                {
                    node.is_deadend = true;
                    node.status = SearchStatus::FAILURE;
                }
                else if (const auto limit = execution.expand(state))
                    return *limit;
            }
            if (node.status == SearchStatus::SUCCESS || node.status == SearchStatus::FAILURE)
            {
                returned = node.status;
                continue;
            }
            node.status = SearchStatus::ACTIVE;
            // Snapshot only the already recorded edges. Later bindings are handled by the active choice stack.
            const auto first_edge = execution.first_outgoing(state);
            stack.push({ state, first_edge });
            if (node.is_open)
                stack.top().result = SearchStatus::FAILURE;
        }
        // Unwind exactly one obligation, leaving the parent's other continuations scheduled.
        if (returned)
        {
            if (stack.empty())
                return *returned == SearchStatus::SUCCESS ? ProgramProofStatus::SUCCESS : ProgramProofStatus::FAILURE;
            stack.top().accept(*returned);
            returned.reset();
        }

        auto& frame = stack.top();
        // Try bindings until this Choose succeeds or exhausts its alternatives.
        if (frame.rule)
        {
            auto& choices = execution.choices();
            if (!choices.empty() && choices.top().state == frame.state)
            {
                if (frame.result != SearchStatus::SUCCESS)
                {
                    if (const auto step = execution.next_binding())
                    {
                        const auto non_singleton = std::visit([](const auto& choice) { return choice.has_alternatives(); }, choices.top().choice);
                        if (const auto limit = execution.record_transition(frame.state, *step, non_singleton))
                            return *limit;
                        next = step->get_target();
                        continue;
                    }
                }
                choices.pop();
            }
            else if (frame.edge != no_edge && execution.predecessors()[frame.edge].rule == frame.rule)
            {
                // A revisited state uses recorded bindings. After success, skip the rest of this rule's group.
                if (frame.result != SearchStatus::SUCCESS)
                    next = execution.predecessors()[frame.edge].target;
                frame.edge = execution.next_outgoing(frame.edge);
                continue;
            }

            // Return the OR result and the next rule's cursor to the enclosing state frame.
            const auto edge = frame.edge;
            returned = frame.result;
            stack.pop();
            stack.top().edge = edge;
            continue;
        }
        // Ordinary outcomes are AND children; recorded Choose groups open their own OR frame.
        if (frame.edge != no_edge)
        {
            const auto& edge = execution.predecessors()[frame.edge];
            if (!edge.rule)
            {
                frame.edge = execution.next_outgoing(frame.edge);
                next = edge.target;
            }
            else
                ygg::visit(
                    [&](auto rule)
                    {
                        if constexpr (ChooseRuleView<decltype(rule)>)
                            stack.push({ frame.state, frame.edge, edge.rule, SearchStatus::FAILURE });
                        else
                        {
                            frame.edge = execution.next_outgoing(frame.edge);
                            next = edge.target;
                        }
                    },
                    edge.rule->get_variant());
            continue;
        }
        // First expansions keep untried Choose denotations on the binding stack, separate from recorded edges.
        if (!execution.choices().empty() && execution.choices().top().state == frame.state)
        {
            std::visit([&](const auto& choice) { stack.push({ frame.state, no_edge, choice.rule, SearchStatus::FAILURE }); }, execution.choices().top().choice);
            continue;
        }
        // Every required continuation has returned. Cache conclusive results, or retain PENDING for later visits.
        execution.search_node(frame.state).status = frame.result;
        returned = frame.result;
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
