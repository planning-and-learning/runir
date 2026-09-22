#ifndef RUNIR_KR_PS_BASE_DL_TRANSITION_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_BASE_DL_TRANSITION_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/base/state_evaluation_context.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::ps::dl
{

template<tyr::TaskKind Kind>
class TransitionEvaluationContext<runir::kr::BaseFamilyTag, Kind> : public BaseTransitionEvaluationContext<runir::kr::BaseFamilyTag, Kind>
{
private:
    using Base = BaseTransitionEvaluationContext<runir::kr::BaseFamilyTag, Kind>;

public:
    TransitionEvaluationContext(tyr::planning::StateView<Kind> source_state,
                                tyr::planning::StateView<Kind> target_state,
                                runir::kr::dl::semantics::Builder& dl_builder,
                                runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository,
                                runir::kr::dl::semantics::EvaluationWorkspace& workspace,
                                runir::kr::dl::semantics::DenotationCaches<runir::kr::BaseFamilyTag>& source_caches,
                                runir::kr::dl::semantics::DenotationCaches<runir::kr::BaseFamilyTag>& target_caches) noexcept :
        Base(std::move(source_state), std::move(target_state), dl_builder, dl_denotation_repository, workspace, source_caches, target_caches)
    {
    }
};

}  // namespace runir::kr::ps::dl

#endif
