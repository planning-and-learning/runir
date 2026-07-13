#ifndef RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_HPP_

#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/sketch_executor_data.hpp"
#include "runir/kr/task_context.hpp"

namespace runir::kr::ps::base
{

template<tyr::planning::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context,
                   SketchView sketch,
                   bool universal = false,
                   const SketchSearchOptions<Kind>& options = SketchSearchOptions<Kind>()) -> SketchProofResults<Kind>;

}  // namespace runir::kr::ps::base

#include "runir/kr/ps/base/sketch_executor_impl.hpp"

#endif
