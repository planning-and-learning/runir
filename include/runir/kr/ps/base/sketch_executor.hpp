#ifndef RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_HPP_

#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/base/declarations.hpp"
#include "runir/kr/ps/base/sketch_executor_data.hpp"

namespace runir::kr::ps::base
{

template<tyr::planning::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context_owner, SketchView sketch, const SketchSearchOptions<Kind>& options) -> SketchProofResults<Kind>;

#ifndef RUNIR_HEADER_INSTANTIATION

extern template auto
find_solution<tyr::planning::GroundTag>(runir::kr::TaskContextPtr<tyr::planning::GroundTag> task_context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::GroundTag>& options) -> SketchProofResults<tyr::planning::GroundTag>;

extern template auto
find_solution<tyr::planning::LiftedTag>(runir::kr::TaskContextPtr<tyr::planning::LiftedTag> task_context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::LiftedTag>& options) -> SketchProofResults<tyr::planning::LiftedTag>;

#endif

}  // namespace runir::kr::ps::base

#ifdef RUNIR_HEADER_INSTANTIATION
#include "runir/kr/ps/base/detail/proof_search.hpp"
#endif

#endif
