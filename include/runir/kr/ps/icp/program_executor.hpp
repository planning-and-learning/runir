#ifndef RUNIR_KR_PS_ICP_PROGRAM_EXECUTOR_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_EXECUTOR_HPP_

#include "runir/kr/ps/icp/program_executor_data.hpp"

namespace runir::kr::ps::icp
{

/// Follow the first admitted successor, or explore all continuations with universal=true.
/// Histories belong to state identity. Every reachable cycle and failing alternative is a failure.
/// Only successful greedy executions return a plan.
template<tyr::TaskKind Kind>
auto find_solution(TaskContextPtr<Kind> task_context, ProgramView program, const ProgramSearchOptions<Kind>& options) -> ProgramProofResults<Kind>;

#ifndef RUNIR_HEADER_INSTANTIATION

extern template auto
find_solution<tyr::GroundTag>(TaskContextPtr<tyr::GroundTag>, ProgramView, const ProgramSearchOptions<tyr::GroundTag>&) -> ProgramProofResults<tyr::GroundTag>;

extern template auto
find_solution<tyr::LiftedTag>(TaskContextPtr<tyr::LiftedTag>, ProgramView, const ProgramSearchOptions<tyr::LiftedTag>&) -> ProgramProofResults<tyr::LiftedTag>;

#endif

}

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/ps/icp/detail/proof_search.hpp"
#endif

#endif
