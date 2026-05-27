#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_IMPL_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_IMPL_HPP_

#include "runir/kr/ps/ext/detail/proof_search.hpp"
#include "runir/kr/ps/ext/detail/solution_search.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"

#include <utility>

namespace runir::kr::ps::ext
{
template<tyr::planning::TaskKind Kind>
auto find_solution(runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options) -> tyr::planning::SearchResult<Kind>
{
    const auto& context = *context_owner;
    const auto execution = detail::execute_solution(context, program, detail::execution_options(options));

    auto result = tyr::planning::SearchResult<Kind> {};
    result.status = detail::translate_execution_status(execution.status);
    if (execution.status == detail::ModuleProgramOutcome::SUCCESS)
    {
        result.goal_node = context.successor_generator->get_node(execution.state.get_index());
        result.plan = execution.plan;
    }
    return result;
}

template<tyr::planning::TaskKind Kind>
auto prove_solution(runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                    ModuleProgramView program,
                    const ModuleProgramSearchOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    const auto& search_context = *context_owner;
    return detail::prove_solution(search_context, std::move(context_owner), program, detail::execution_options(options));
}

}  // namespace runir::kr::ps::ext

#endif
