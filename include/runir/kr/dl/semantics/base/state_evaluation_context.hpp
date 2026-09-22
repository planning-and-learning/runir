#ifndef RUNIR_KR_DL_SEMANTICS_BASE_STATE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_BASE_STATE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

template<tyr::TaskKind Kind>
class StateEvaluationContext<runir::kr::BaseFamilyTag, Kind> : public BaseStateEvaluationContext<runir::kr::BaseFamilyTag, Kind>
{
private:
    using Base = BaseStateEvaluationContext<runir::kr::BaseFamilyTag, Kind>;

public:
    StateEvaluationContext(tyr::planning::StateView<Kind> state,
                           Builder& builder,
                           DenotationRepository& denotation_repository,
                           EvaluationWorkspace& workspace,
                           DenotationCaches<runir::kr::BaseFamilyTag>& caches) noexcept :
        Base(std::move(state), builder, denotation_repository, workspace, caches)
    {
    }
};

}  // namespace runir::kr::dl::semantics

#endif
