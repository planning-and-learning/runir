#include "runir/kr/ps/base/sketch_executor.hpp"

namespace runir::kr::ps::base
{

template class EvaluationContext<tyr::planning::GroundTag>;
template class EvaluationContext<tyr::planning::LiftedTag>;
template class SuccessorExpander<tyr::planning::GroundTag>;
template class SuccessorExpander<tyr::planning::LiftedTag>;

template auto
find_solution<tyr::planning::GroundTag>(runir::kr::TaskContextPtr<tyr::planning::GroundTag> task_context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::GroundTag>& options) -> SketchProofResults<tyr::planning::GroundTag>;

template auto
find_solution<tyr::planning::LiftedTag>(runir::kr::TaskContextPtr<tyr::planning::LiftedTag> task_context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::planning::LiftedTag>& options) -> SketchProofResults<tyr::planning::LiftedTag>;

}  // namespace runir::kr::ps::base
