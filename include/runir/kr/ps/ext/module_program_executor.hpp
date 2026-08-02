#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"

namespace runir::kr::ps::ext
{
template<tyr::planning::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options) -> ModuleProgramProofResults<Kind>;

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

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/ps/ext/detail/proof_search.hpp"
#endif

#endif
