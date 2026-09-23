#ifndef RUNIR_KR_PS_EXT_DETAIL_GREEDY_EXECUTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_GREEDY_EXECUTION_HPP_

#include "runir/kr/ps/ext/detail/execution_state.hpp"

#include <concepts>
#include <optional>
#include <type_traits>
#include <variant>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
using ExecutionOutcome = std::variant<PendingChoice<Kind>, ProgramProofStatus>;

template<tyr::TaskKind Kind, typename Unsolvability>
std::optional<ProgramProofStatus>
apply_greedy_step(ExecutionState<Kind, Unsolvability>& execution, ProgramStateView<Kind> source, const ProgramStep<Kind>& step)
{
    if (step.status == ProgramOutcome::OUT_OF_TIME)
        return ProgramProofStatus::OUT_OF_TIME;
    if (step.status == ProgramOutcome::OUT_OF_STATES)
        return ProgramProofStatus::OUT_OF_STATES;
    if (step.status == ProgramOutcome::APPLIED || step.status == ProgramOutcome::RESTORED_CALLER)
        return execution.record_transition(source, step, 0);
    execution.mark_open(source.get_index());
    return std::nullopt;
}

/// Execute ordinary work until a pending choice, a completed attempt, or a resource limit.
template<tyr::TaskKind Kind, typename Unsolvability>
ExecutionOutcome<Kind> run_greedy(ExecutionState<Kind, Unsolvability>& execution)
{
    if (!execution.initialize())
        return ProgramProofStatus::OUT_OF_STATES;
    if (execution.out_of_time())
        return ProgramProofStatus::OUT_OF_TIME;

    while (execution.has_pending())
    {
        if (execution.out_of_time())
            return ProgramProofStatus::OUT_OF_TIME;

        const auto pending = execution.pop();
        if (const auto* choice = std::get_if<PendingChoice<Kind>>(&pending))
        {
            if (execution.out_of_time())
                return ProgramProofStatus::OUT_OF_TIME;
            return *choice;
        }
        const auto state = execution.state_view(std::get<ygg::Index<ProgramState<Kind>>>(pending));
        auto& node = execution.search_node(state.get_index());
        if (node.is_goal)
        {
            if (!execution.universal())
            {
                execution.select_goal(state.get_index());
                return ProgramProofStatus::SUCCESS;
            }
            continue;
        }
        if (node.is_unsolvable)
        {
            execution.mark_deadend(state.get_index());
            continue;
        }

        ++execution.statistics().num_expanded;
        auto limit = std::optional<ProgramProofStatus> {};
        execution.expander().for_each_successor(
            state,
            tyr::planning::Node<Kind>(state.get_state(), node.metric),
            execution.statistics(),
            [&](const typename SuccessorExpander<Kind>::Expansion& expansion)
            {
                if (execution.out_of_time())
                {
                    limit = ProgramProofStatus::OUT_OF_TIME;
                    return false;
                }
                std::visit(
                    [&](const auto& value)
                    {
                        if constexpr (std::same_as<std::decay_t<decltype(value)>, ProgramStep<Kind>>)
                            limit = apply_greedy_step(execution, state, value);
                        else
                            execution.push(PendingChoice<Kind> { state.get_index(), value });
                    },
                    expansion);
                return !limit && execution.universal();
            },
            [&] { return execution.out_of_time(); });
        if (limit)
            return *limit;
        if (execution.out_of_time())
            return ProgramProofStatus::OUT_OF_TIME;
    }

    if (execution.out_of_time())
        return ProgramProofStatus::OUT_OF_TIME;
    return execution.assess_attempt();
}

}  // namespace runir::kr::ps::ext::detail

#endif
