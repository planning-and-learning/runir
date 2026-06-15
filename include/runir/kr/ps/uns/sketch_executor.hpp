#ifndef RUNIR_KR_PS_UNS_SKETCH_EXECUTOR_HPP_
#define RUNIR_KR_PS_UNS_SKETCH_EXECUTOR_HPP_

#include "runir/datasets/task_class.hpp"
#include "runir/kr/ps/uns/repository.hpp"
#include "runir/kr/ps/uns/sketch_executor_data.hpp"

namespace runir::kr::ps::uns
{

template<tyr::planning::TaskKind Kind>
auto prove_solution(runir::datasets::TaskSearchContextPtr<Kind> context,
                    SketchView sketch,
                    const SketchSearchOptions<Kind>& options = SketchSearchOptions<Kind>()) -> SketchProofResults<Kind>;

template<tyr::planning::TaskKind Kind>
auto find_solution(runir::datasets::TaskSearchContextPtr<Kind> context,
                   SketchView sketch,
                   const SketchSearchOptions<Kind>& options = SketchSearchOptions<Kind>()) -> SketchProofResults<Kind>;

}  // namespace runir::kr::ps::uns

#include "runir/kr/ps/uns/sketch_executor_impl.hpp"

#endif
