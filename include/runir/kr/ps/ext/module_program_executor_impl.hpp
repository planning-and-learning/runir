#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_IMPL_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_IMPL_HPP_

#include "runir/kr/ps/ext/detail/proof_search.hpp"
#include "runir/kr/ps/ext/detail/transition_search.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"

#include <utility>

namespace runir::kr::ps::ext
{
template<tyr::planning::TaskKind Kind>
auto find_solution(runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    const auto& search_context = *context_owner;
    return detail::find_solution(search_context, std::move(context_owner), program, detail::execution_options(options));
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
