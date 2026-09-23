#ifndef RUNIR_KR_PS_EXT_DETAIL_GREEDY_EXECUTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_GREEDY_EXECUTION_HPP_

#include "runir/kr/ps/ext/detail/execution_state.hpp"

#include <concepts>
#include <optional>
#include <type_traits>
#include <variant>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind, typename Unsolvability>
std::optional<ProgramProofStatus>
apply_greedy_step(ExecutionState<Kind, Unsolvability>& execution, ProgramStateView<Kind> source, const ProgramStep<Kind>& step)
{
    if (step.status == ProgramOutcome::OUT_OF_TIME)
        return ProgramProofStatus::OUT_OF_TIME;
    if (step.status == ProgramOutcome::OUT_OF_STATES)
        return ProgramProofStatus::OUT_OF_STATES;
    if (step.status == ProgramOutcome::APPLIED || step.status == ProgramOutcome::RESTORED_CALLER)
        return execution.record_transition(source, step);
    execution.mark_open(source.get_index());
    return std::nullopt;
}

/// Enumerate one state before descending: successor generation is not reentrant.
template<tyr::TaskKind Kind, typename Unsolvability>
std::optional<ProgramProofStatus> expand_state(ExecutionState<Kind, Unsolvability>& execution, ygg::Index<ProgramState<Kind>> index)
{
    const auto state = execution.state_view(index);
    ++execution.statistics().num_expanded;
    auto limit = std::optional<ProgramProofStatus> {};
    execution.expander().for_each_successor(
        state,
        tyr::planning::Node<Kind>(state.get_state(), 0),
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
                        execution.add_choice(index, value);
                },
                expansion);
            return !limit && execution.universal();
        },
        [&] { return execution.out_of_time(); });
    if (limit)
        return limit;
    if (execution.out_of_time())
        return ProgramProofStatus::OUT_OF_TIME;
    return std::nullopt;
}

}  // namespace runir::kr::ps::ext::detail

#endif
