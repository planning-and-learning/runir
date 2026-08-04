#include "runir/kr/ps/base/sketch_executor.hpp"

#include "runir/kr/ps/base/detail/proof_search.hpp"

namespace runir::kr::ps::base
{

template class EvaluationContext<tyr::GroundTag>;
template class EvaluationContext<tyr::LiftedTag>;
template class SuccessorExpander<tyr::GroundTag>;
template class SuccessorExpander<tyr::LiftedTag>;

template auto
find_solution<tyr::GroundTag>(runir::kr::TaskContextPtr<tyr::GroundTag> task_context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::GroundTag>& options) -> SketchProofResults<tyr::GroundTag>;

template auto
find_solution<tyr::LiftedTag>(runir::kr::TaskContextPtr<tyr::LiftedTag> task_context,
                                        SketchView sketch,
                                        const SketchSearchOptions<tyr::LiftedTag>& options) -> SketchProofResults<tyr::LiftedTag>;

}  // namespace runir::kr::ps::base
