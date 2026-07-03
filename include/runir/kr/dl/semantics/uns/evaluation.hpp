#ifndef RUNIR_KR_DL_SEMANTICS_UNS_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_UNS_EVALUATION_HPP_

#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/uns/evaluation_context.hpp"

namespace runir::kr::dl::semantics::uns
{

template<tyr::planning::TaskKind Kind>
const auto& get_repository(const runir::kr::dl::semantics::EvaluationContext<runir::kr::UnsFamilyTag, Kind>& context) noexcept
{
    return runir::kr::dl::semantics::get_repository(context);
}

using runir::kr::dl::semantics::evaluate;
using runir::kr::dl::semantics::evaluate_impl;

}  // namespace runir::kr::dl::semantics::uns

#endif
