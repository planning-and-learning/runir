#ifndef RUNIR_KR_PS_ICP_DL_TRANSITION_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_ICP_DL_TRANSITION_EVALUATION_CONTEXT_HPP_

#include "runir/kr/ps/ext/dl/transition_evaluation_context.hpp"

namespace runir::kr::ps::dl
{

template<tyr::TaskKind Kind>
class TransitionEvaluationContext<runir::kr::IcpFamilyTag, Kind> : public TransitionEvaluationContext<runir::kr::ExtFamilyTag, Kind>
{
    using Base = TransitionEvaluationContext<runir::kr::ExtFamilyTag, Kind>;

public:
    using Base::Base;
};

}

#endif
