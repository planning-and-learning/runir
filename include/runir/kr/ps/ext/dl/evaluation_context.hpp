#ifndef RUNIR_KR_PS_EXT_DL_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_EXT_DL_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/ext/evaluation_context.hpp"
#include "runir/kr/ps/dl/evaluation_context.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>

namespace runir::kr::ps::dl
{

template<tyr::planning::TaskKind Kind>
class EvaluationContext<runir::kr::ExtFamilyTag, Kind> :
    public BaseEvaluationContext<EvaluationContext<runir::kr::ExtFamilyTag, Kind>, runir::kr::ExtFamilyTag, Kind>
{
private:
    using Base = BaseEvaluationContext<EvaluationContext<runir::kr::ExtFamilyTag, Kind>, runir::kr::ExtFamilyTag, Kind>;

public:
    using Base::Base;
};

}  // namespace runir::kr::ps::dl

namespace runir::kr::ps::ext::dl
{

template<tyr::planning::TaskKind Kind>
using EvaluationContext = runir::kr::ps::dl::EvaluationContext<runir::kr::ExtFamilyTag, Kind>;

}  // namespace runir::kr::ps::ext::dl

#endif
