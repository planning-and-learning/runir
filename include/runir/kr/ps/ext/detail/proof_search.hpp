#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/choice_execution.hpp"
#include "runir/kr/ps/ext/program_executor.hpp"
#include "runir/kr/ps/unsolvability.hpp"

#include <utility>

namespace runir::kr::ps::ext
{
namespace detail
{

template<tyr::TaskKind Kind, typename Unsolvability>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program, const ProgramSearchOptions<Kind>& options, Unsolvability& classifier)
    -> ProgramProofResults<Kind>
{
    auto execution = ExecutionState<Kind, Unsolvability>(std::move(task_context), program, options, classifier);
    if (!execution.initialize())
        return execution.finish(ProgramProofStatus::OUT_OF_STATES);
    return execution.finish(run_execution(execution));
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
