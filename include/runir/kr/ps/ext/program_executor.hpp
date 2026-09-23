#ifndef RUNIR_KR_PS_EXT_PROGRAM_EXECUTOR_HPP_
#define RUNIR_KR_PS_EXT_PROGRAM_EXECUTOR_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/ext/declarations.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"

namespace runir::kr::ps::ext
{
/// Execute greedily, or require all ordinary continuations when options.universal is true.
/// Every selected Choose rule needs one successful binding; distinct Choose rules remain separate obligations.
/// Each program state is expanded at most once, and limits count work across all attempted bindings.
/// The returned graph includes rejected branches; only successful non-universal searches return a plan.
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
