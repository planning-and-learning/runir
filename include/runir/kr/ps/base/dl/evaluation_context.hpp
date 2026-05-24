#ifndef RUNIR_KR_PS_BASE_DL_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_BASE_DL_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/base/evaluation_context.hpp"
#include "runir/kr/ps/dl/evaluation_context.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>

namespace runir::kr::ps::dl
{

template<tyr::planning::TaskKind Kind>
class EvaluationContext<runir::kr::BaseFamilyTag, Kind> :
    public BaseEvaluationContext<EvaluationContext<runir::kr::BaseFamilyTag, Kind>, runir::kr::BaseFamilyTag, Kind>
{
private:
    using Base = BaseEvaluationContext<EvaluationContext<runir::kr::BaseFamilyTag, Kind>, runir::kr::BaseFamilyTag, Kind>;

public:
    using Base::Base;
};

}  // namespace runir::kr::ps::dl

namespace runir::kr::ps::base::dl
{

template<tyr::planning::TaskKind Kind>
using EvaluationContext = runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, Kind>;

}  // namespace runir::kr::ps::base::dl

#endif
