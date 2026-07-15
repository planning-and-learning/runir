#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/ext/detail/proof_search.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <utility>

namespace runir::kr::ps::ext
{
template<tyr::planning::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    return detail::find_solution(std::move(task_context), program, options);
}

#ifndef RUNIR_HEADER_INSTANTIATION

extern template auto find_solution<tyr::planning::GroundTag>(runir::kr::TaskContextPtr<tyr::planning::GroundTag> task_context,
                                                             ModuleProgramView program,
                                                             const ModuleProgramSearchOptions<tyr::planning::GroundTag>& options)
    -> ModuleProgramProofResults<tyr::planning::GroundTag>;

extern template auto find_solution<tyr::planning::LiftedTag>(runir::kr::TaskContextPtr<tyr::planning::LiftedTag> task_context,
                                                             ModuleProgramView program,
                                                             const ModuleProgramSearchOptions<tyr::planning::LiftedTag>& options)
    -> ModuleProgramProofResults<tyr::planning::LiftedTag>;

#endif

}  // namespace runir::kr::ps::ext

#endif
