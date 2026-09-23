#ifndef RUNIR_KR_PS_EXT_DETAIL_CHOICE_EXECUTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_CHOICE_EXECUTION_HPP_

#include "runir/kr/ps/ext/detail/execution_state.hpp"

#include <optional>
#include <utility>
#include <variant>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
struct ChoiceFrame
{
    PendingChoice<Kind> pending;
    ExecutionCheckpoint<Kind> checkpoint;
};

/// Begin a choice after its pending work has been popped, retaining the remaining siblings for retries.
template<tyr::TaskKind Kind, typename Unsolvability>
ChoiceFrame<Kind> enter_choice(ExecutionState<Kind, Unsolvability>& execution, PendingChoice<Kind> pending)
{
    execution.statistics().num_choice_points += std::visit([](const auto& choice) { return choice.has_alternatives(); }, pending.choice);
    return { std::move(pending), execution.checkpoint() };
}

/// Apply only the current binding; ordinary execution resumes separately in the proof-search driver.
template<tyr::TaskKind Kind, typename Unsolvability>
std::optional<ProgramProofStatus> try_choice(ExecutionState<Kind, Unsolvability>& execution, const ChoiceFrame<Kind>& frame)
{
    if (execution.out_of_time())
        return ProgramProofStatus::OUT_OF_TIME;
    const auto source = execution.state_view(frame.pending.state);
    const auto& node = execution.search_node(frame.pending.state);
    return std::visit(
        [&](const auto& choice) -> std::optional<ProgramProofStatus>
        {
            const auto step = execution.expander().apply_choice(source,
                                                               tyr::planning::Node<Kind>(source.get_state(), node.metric),
                                                               choice,
                                                               execution.statistics());
            if (step.status == ProgramOutcome::APPLIED)
            {
                ++execution.statistics().num_binding_attempts;
                return execution.record_transition(source, step, ygg::uint_t(choice.has_alternatives()));
            }
            execution.mark_deadend(frame.pending.state);
            return std::nullopt;
        },
        frame.pending.choice);
}

/// Restore the newest choice's continuation and advance its binding, returning false when exhausted.
template<tyr::TaskKind Kind, typename Unsolvability>
bool advance_choice(ExecutionState<Kind, Unsolvability>& execution, ChoiceFrame<Kind>& frame)
{
    return std::visit(
        [&](auto& choice)
        {
            execution.statistics().num_backtracks += !choice.exhausted();
            execution.restore(frame.checkpoint);
            if (!choice.exhausted())
                choice.advance();
            return !choice.exhausted();
        },
        frame.pending.choice);
}

}  // namespace runir::kr::ps::ext::detail

#endif
