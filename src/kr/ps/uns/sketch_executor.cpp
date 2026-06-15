#include "runir/kr/ps/uns/sketch_executor_impl.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION
namespace runir::kr::ps::uns
{

template auto
prove_solution<tyr::planning::GroundTag>(datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context,
                                         SketchView sketch,
                                         const SketchSearchOptions<tyr::planning::GroundTag>& options) -> SketchProofResults<tyr::planning::GroundTag>;

template auto
prove_solution<tyr::planning::LiftedTag>(datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context,
                                         SketchView sketch,
                                         const SketchSearchOptions<tyr::planning::LiftedTag>& options) -> SketchProofResults<tyr::planning::LiftedTag>;

template auto
find_solution<tyr::planning::GroundTag>(datasets::TaskSearchContextPtr<tyr::planning::GroundTag> context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::GroundTag>& options) -> SketchProofResults<tyr::planning::GroundTag>;

template auto
find_solution<tyr::planning::LiftedTag>(datasets::TaskSearchContextPtr<tyr::planning::LiftedTag> context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::LiftedTag>& options) -> SketchProofResults<tyr::planning::LiftedTag>;

}  // namespace runir::kr::ps::uns
#endif
