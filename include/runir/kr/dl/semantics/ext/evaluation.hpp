#ifndef RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"

namespace runir::kr::dl::semantics::ext
{

using FamilyTag = runir::kr::ExtFamilyTag;

template<tyr::planning::TaskKind Kind>
using EvaluationContext = runir::kr::dl::semantics::EvaluationContext<FamilyTag, Kind>;

template<tyr::planning::TaskKind Kind>
const auto& get_repository(const EvaluationContext<Kind>& context) noexcept
{
    return runir::kr::dl::semantics::get_repository(context);
}

using runir::kr::dl::semantics::evaluate;
using runir::kr::dl::semantics::evaluate_impl;

}  // namespace runir::kr::dl::semantics::ext

#endif
