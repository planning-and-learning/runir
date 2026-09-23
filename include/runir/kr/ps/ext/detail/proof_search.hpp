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

template<tyr::TaskKind Kind>
struct SearchFrame
{
    ProgramStateView<Kind> state;
    std::size_t edge;
    // A state frame requires every rule; a rule frame combines its own continuations.
    std::optional<RuleVariantView> rule = std::nullopt;
    SearchStatus result = SearchStatus::SUCCESS;

    // Ordinary continuations are conjunctive; one pending child cannot override a definite failure.
    void require(SearchStatus child)
    {
        if (child == SearchStatus::FAILURE || (child == SearchStatus::PENDING && result == SearchStatus::SUCCESS))
            result = child;
    }

    void accept(SearchStatus child)
    {
        if (!rule)
            require(child);
        else
            ygg::visit(
                [&](auto concrete)
                {
                    if constexpr (ChooseRuleView<decltype(concrete)>)
                    {
                        if (child == SearchStatus::SUCCESS || (child == SearchStatus::PENDING && result == SearchStatus::FAILURE))
                            result = child;
                    }
                    else
                        require(child);
                },
                rule->get_variant());
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
    const auto admitted = execution.discover(initial);
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

    constexpr auto no_edge = ExecutionState<Kind, Unsolvability>::no_edge;
    auto stack = std::stack<SearchFrame<Kind>, std::vector<SearchFrame<Kind>>> {};
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
                    return finish(*limit);
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
        if (returned)
        {
            if (stack.empty())
                return finish(*returned == SearchStatus::SUCCESS ? ProgramProofStatus::SUCCESS : ProgramProofStatus::FAILURE);
            stack.top().accept(*returned);
            returned.reset();
        }

        auto& frame = stack.top();
        if (frame.rule)
        {
            const auto limit = ygg::visit(
                [&](auto rule) -> std::optional<ProgramProofStatus>
                {
                    if constexpr (ChooseRuleView<decltype(rule)>)
                    {
                        if (!execution.choices().empty() && execution.choices().top().state == frame.state)
                        {
                            if (frame.result != SearchStatus::SUCCESS)
                                if (const auto step = execution.next_binding())
                                {
                                    if (step->status == ProgramOutcome::OUT_OF_TIME)
                                        return ProgramProofStatus::OUT_OF_TIME;
                                    if (step->status == ProgramOutcome::OUT_OF_STATES)
                                        return ProgramProofStatus::OUT_OF_STATES;
                                    if (step->status == ProgramOutcome::APPLIED)
                                    {
                                        const auto non_singleton =
                                            std::visit([](const auto& choice) { return choice.has_alternatives(); }, execution.choices().top().choice);
                                        if (const auto limit = execution.record_transition(frame.state, *step, non_singleton))
                                            return limit;
                                        next = step->get_target();
                                        return std::nullopt;
                                    }
                                    execution.search_node(frame.state).is_deadend = true;
                                }
                            execution.choices().pop();
                            return std::nullopt;
                        }
                        if (frame.result == SearchStatus::SUCCESS)
                            return std::nullopt;
                    }
                    if (frame.edge != no_edge && execution.predecessors()[frame.edge].rule == frame.rule)
                    {
                        next = execution.predecessors()[frame.edge].target;
                        frame.edge = execution.next_outgoing(frame.edge);
                    }
                    return std::nullopt;
                },
                frame.rule->get_variant());
            if (limit)
                return finish(*limit);
            if (next)
                continue;

            // Skip untried recorded bindings after a successful Choose, then resume the parent state's next rule.
            while (frame.edge != no_edge && execution.predecessors()[frame.edge].rule == frame.rule)
                frame.edge = execution.next_outgoing(frame.edge);
            const auto edge = frame.edge;
            returned = frame.result;
            stack.pop();
            stack.top().edge = edge;
            continue;
        }
        if (frame.edge != no_edge)
        {
            // A rule frame consumes one consecutive group and returns the next group's cursor to this state frame.
            const auto& edge = execution.predecessors()[frame.edge];
            if (!edge.rule)
            {
                frame.edge = execution.next_outgoing(frame.edge);
                next = edge.target;
            }
            else
                ygg::visit(
                    [&](auto rule)
                    { stack.push({ frame.state, frame.edge, edge.rule, ChooseRuleView<decltype(rule)> ? SearchStatus::FAILURE : SearchStatus::SUCCESS }); },
                    edge.rule->get_variant());
            continue;
        }
        if (!execution.choices().empty() && execution.choices().top().state == frame.state)
        {
            std::visit([&](const auto& choice) { stack.push({ frame.state, no_edge, choice.rule, SearchStatus::FAILURE }); }, execution.choices().top().choice);
            continue;
        }
        execution.search_node(frame.state).status = frame.result;
        returned = frame.result;
        stack.pop();
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
