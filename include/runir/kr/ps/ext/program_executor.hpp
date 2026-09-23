#ifndef RUNIR_KR_PS_EXT_PROGRAM_EXECUTOR_HPP_
#define RUNIR_KR_PS_EXT_PROGRAM_EXECUTOR_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"

namespace runir::kr::ps::ext
{
template<tyr::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context,
                   ProgramView program,
                   const ProgramSearchOptions<Kind>& options) -> ProgramProofResults<Kind>;

#ifndef RUNIR_HEADER_INSTANTIATION

extern template auto find_solution<tyr::GroundTag>(runir::kr::TaskContextPtr<tyr::GroundTag> task_context,
                                                             ProgramView program,
                                                             const ProgramSearchOptions<tyr::GroundTag>& options)
    -> ProgramProofResults<tyr::GroundTag>;

extern template auto find_solution<tyr::LiftedTag>(runir::kr::TaskContextPtr<tyr::LiftedTag> task_context,
                                                             ProgramView program,
                                                             const ProgramSearchOptions<tyr::LiftedTag>& options)
    -> ProgramProofResults<tyr::LiftedTag>;

#endif

}  // namespace runir::kr::ps::ext

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/ps/ext/detail/proof_search.hpp"
#endif

#endif
