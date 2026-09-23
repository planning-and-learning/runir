#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/choice_execution.hpp"
#include "runir/kr/ps/ext/detail/greedy_execution.hpp"
#include "runir/kr/ps/ext/program_executor.hpp"
#include "runir/kr/ps/unsolvability.hpp"

#include <utility>
#include <variant>
#include <vector>

namespace runir::kr::ps::ext
{
namespace detail
{

template<tyr::TaskKind Kind, typename Unsolvability>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program, const ProgramSearchOptions<Kind>& options, Unsolvability& classifier)
    -> ProgramProofResults<Kind>
{
    auto execution = ExecutionState<Kind, Unsolvability>(std::move(task_context), program, options, classifier);
    auto choices = std::vector<ChoiceFrame<Kind>> {};
    const auto resume = [&](const ChoiceFrame<Kind>& frame) -> ExecutionOutcome<Kind>
    {
        if (const auto status = try_choice(execution, frame))
            return *status;
        return run_greedy(execution);
    };
    auto outcome = run_greedy(execution);
    while (true)
    {
        if (auto* choice = std::get_if<PendingChoice<Kind>>(&outcome))
        {
            choices.push_back(enter_choice(execution, std::move(*choice)));
            outcome = resume(choices.back());
            continue;
        }

        const auto status = std::get<ProgramProofStatus>(outcome);
        if (status != ProgramProofStatus::FAILURE)
            return execution.finish(status);

        while (!choices.empty() && !advance_choice(execution, choices.back()))
            choices.pop_back();
        if (choices.empty())
            return execution.finish(ProgramProofStatus::FAILURE);
        outcome = resume(choices.back());
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
