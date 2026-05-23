#include "runir/kr/ps/base/sketch_executor_impl.hpp"

#ifndef RUNIR_HEADER_INSTANTIATION
namespace runir::kr::ps::base
{

template auto
prove_solution<tyr::planning::GroundTag>(const datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
                                         SketchView sketch,
                                         const SketchSearchOptions<tyr::planning::GroundTag>& options) -> SketchProofResults<tyr::planning::GroundTag>;

template auto
prove_solution<tyr::planning::LiftedTag>(const datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
                                         SketchView sketch,
                                         const SketchSearchOptions<tyr::planning::LiftedTag>& options) -> SketchProofResults<tyr::planning::LiftedTag>;

template auto
find_solution<tyr::planning::GroundTag>(const datasets::TaskSearchContext<tyr::planning::GroundTag>& context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::GroundTag>& options) -> tyr::planning::SearchResult<tyr::planning::GroundTag>;

template auto
find_solution<tyr::planning::LiftedTag>(const datasets::TaskSearchContext<tyr::planning::LiftedTag>& context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::LiftedTag>& options) -> tyr::planning::SearchResult<tyr::planning::LiftedTag>;

}  // namespace runir::kr::ps::base
#endif
