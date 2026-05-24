#ifndef RUNIR_KR_PS_BASE_DL_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_BASE_DL_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/base/evaluation_context.hpp"
#include "runir/kr/ps/dl/evaluation_context.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::ps::dl
{

template<tyr::planning::TaskKind Kind>
class EvaluationContext<runir::kr::BaseFamilyTag, Kind> :
    public BaseEvaluationContext<EvaluationContext<runir::kr::BaseFamilyTag, Kind>, runir::kr::BaseFamilyTag, Kind>
{
private:
    using Base = BaseEvaluationContext<EvaluationContext<runir::kr::BaseFamilyTag, Kind>, runir::kr::BaseFamilyTag, Kind>;

public:
    EvaluationContext(tyr::planning::StateView<Kind> source_state,
                      tyr::planning::StateView<Kind> target_state,
                      runir::kr::dl::semantics::Builder& dl_builder,
                      runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository) noexcept :
        Base(std::move(source_state), std::move(target_state), dl_builder, dl_denotation_repository)
    {
    }
};

}  // namespace runir::kr::ps::dl

namespace runir::kr::ps::base::dl
{

template<tyr::planning::TaskKind Kind>
using EvaluationContext = runir::kr::ps::dl::EvaluationContext<runir::kr::BaseFamilyTag, Kind>;

}  // namespace runir::kr::ps::base::dl

#endif
