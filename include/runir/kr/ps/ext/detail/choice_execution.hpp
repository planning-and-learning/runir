#ifndef RUNIR_KR_PS_EXT_DETAIL_CHOICE_EXECUTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_CHOICE_EXECUTION_HPP_

#include "runir/kr/ps/ext/detail/greedy_execution.hpp"

#include <optional>
#include <variant>
#include <vector>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind, typename Unsolvability>
std::optional<ProgramProofStatus> try_next_choice(ExecutionState<Kind, Unsolvability>& execution, std::size_t index)
{
    auto& frame = execution.choices()[index];
    const auto source = frame.state;
    return std::visit(
        [&](auto& choice) -> std::optional<ProgramProofStatus>
        {
            if (choice.exhausted())
                return ProgramProofStatus::FAILURE;
            const auto step = execution.expander().apply_choice(source, tyr::planning::Node<Kind>(source.get_state(), 0), choice, execution.statistics());
            choice.advance();
            if (step.status == ProgramOutcome::OUT_OF_TIME)
                return ProgramProofStatus::OUT_OF_TIME;
            if (step.status == ProgramOutcome::OUT_OF_STATES)
                return ProgramProofStatus::OUT_OF_STATES;
            if (step.status == ProgramOutcome::APPLIED)
                return execution.record_transition(source, step, choice.has_alternatives());
            execution.mark_deadend(frame.state);
            return ProgramProofStatus::FAILURE;
        },
        frame.choice);
}

template<tyr::TaskKind Kind>
struct SearchFrame
{
    ProgramStateView<Kind> state;
    std::size_t successors_begin;
    std::size_t successor;
    std::size_t choices_begin;
    std::size_t choice;
    bool replay;
    std::size_t binding = 0;
    SearchStatus result = SearchStatus::SUCCESS;
    SearchStatus choice_result = SearchStatus::NEW;

    // Ordinary continuations are conjunctive; one pending child cannot override a definite failure.
    void require(SearchStatus child)
    {
        if (child == SearchStatus::FAILURE || (child == SearchStatus::PENDING && result == SearchStatus::SUCCESS))
            result = child;
    }

    void accept(SearchStatus child)
    {
        if (choice_result == SearchStatus::NEW)
            require(child);
        else if (child == SearchStatus::SUCCESS)
        {
            --choice;
            choice_result = SearchStatus::NEW;
        }
        else if (child == SearchStatus::PENDING)
            choice_result = child;
    }
};

/// DFS establishes completion while unwinding. Only pending cyclic dependencies need another walk of recorded edges.
template<tyr::TaskKind Kind, typename Unsolvability>
ProgramProofStatus run_execution(ExecutionState<Kind, Unsolvability>& execution)
{
    auto stack = std::vector<SearchFrame<Kind>> {};
    auto next = std::optional(execution.initial());
    auto returned = std::optional<SearchStatus> {};
    while (true)
    {
        if (execution.out_of_time())
            return ProgramProofStatus::OUT_OF_TIME;
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
                    execution.select_goal(state);
                    node.status = SearchStatus::SUCCESS;
                }
                else if (node.is_unsolvable)
                {
                    execution.mark_deadend(state);
                    node.status = SearchStatus::FAILURE;
                }
                else if (const auto limit = expand_state(execution, state))
                    return *limit;
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
                return *returned == SearchStatus::SUCCESS ? ProgramProofStatus::SUCCESS : ProgramProofStatus::FAILURE;
            stack.back().accept(*returned);
            returned.reset();
        }

        auto& frame = stack.back();
        auto& node = execution.search_node(frame.state);
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
        if (frame.choice != frame.choices_begin)
        {
            const auto index = frame.choice - 1;
            if (execution.choices()[index].state != frame.state)
            {
                --frame.choice;
                continue;
            }
            if (frame.choice_result == SearchStatus::NEW)
            {
                frame.choice_result = SearchStatus::FAILURE;
                // ponytail: pending cyclic revisits scan existing records; add an outgoing index only if profiling warrants it.
                frame.binding = frame.replay ? execution.predecessors().size() : 0;
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
                        execution.choices()[index].choice);
                continue;
            }
            const auto status = try_next_choice(execution, index);
            if (!status)
            {
                next = execution.predecessors().back().target;
                continue;
            }
            if (*status != ProgramProofStatus::FAILURE)
                return *status;
            frame.require(frame.choice_result);
            frame.choice_result = SearchStatus::NEW;
            --frame.choice;
            continue;
        }
        node.status = frame.result;
        returned = frame.result;
        stack.pop_back();
    }
}

}  // namespace runir::kr::ps::ext::detail

#endif
