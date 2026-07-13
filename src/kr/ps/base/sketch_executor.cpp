#include "runir/kr/ps/base/sketch_executor_impl.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION
namespace runir::kr::ps::base
{

template auto
find_solution<tyr::planning::GroundTag>(runir::kr::TaskContextPtr<tyr::planning::GroundTag> task_context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::GroundTag>& options) -> SketchProofResults<tyr::planning::GroundTag>;

template auto
find_solution<tyr::planning::LiftedTag>(runir::kr::TaskContextPtr<tyr::planning::LiftedTag> task_context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::LiftedTag>& options) -> SketchProofResults<tyr::planning::LiftedTag>;

}  // namespace runir::kr::ps::base
#endif
