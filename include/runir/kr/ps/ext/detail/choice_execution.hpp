#ifndef RUNIR_KR_PS_EXT_DETAIL_CHOICE_EXECUTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_CHOICE_EXECUTION_HPP_

#include "runir/kr/ps/ext/detail/execution_state.hpp"

#include <optional>
#include <variant>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind, typename Unsolvability>
std::optional<ProgramProofStatus> try_next_choice(ExecutionState<Kind, Unsolvability>& execution)
{
    auto& choices = execution.choices();
    // Give pending obligations their first binding before retrying choices that may depend on them.
    for (const auto untouched_only : { true, false })
        for (std::size_t i = choices.size(); i-- > 0;)
        {
            if (execution.out_of_time())
                return ProgramProofStatus::OUT_OF_TIME;
            auto& frame = choices[i];
            if (execution.choice_is_proved(i))
                continue;
            const auto eligible =
                std::visit([&](const auto& choice) { return !choice.exhausted() && (!untouched_only || choice.cursor == choice.denotation.begin()); },
                           frame.choice);
            if (!eligible)
                continue;

            const auto source = execution.state_view(frame.state);
            return std::visit(
                [&](auto& choice) -> std::optional<ProgramProofStatus>
                {
                    const auto step =
                        execution.expander().apply_choice(source, tyr::planning::Node<Kind>(source.get_state(), 0), choice, execution.statistics());
                    choice.advance();
                    if (step.status == ProgramOutcome::APPLIED)
                    {
                        return execution.record_transition(source, step, i);
                    }
                    execution.mark_deadend(frame.state);
                    return std::nullopt;
                },
                frame.choice);
        }
    return ProgramProofStatus::FAILURE;
}

}  // namespace runir::kr::ps::ext::detail

#endif
