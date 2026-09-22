#ifndef RUNIR_KR_PS_EXT_DL_TRANSITION_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_EXT_DL_TRANSITION_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/ext/state_evaluation_context.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::ps::dl
{

template<tyr::TaskKind Kind>
class TransitionEvaluationContext<runir::kr::ExtFamilyTag, Kind> : public BaseTransitionEvaluationContext<runir::kr::ExtFamilyTag, Kind>
{
private:
    using Base = BaseTransitionEvaluationContext<runir::kr::ExtFamilyTag, Kind>;

public:
    using DlContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, Kind>;

    TransitionEvaluationContext(tyr::planning::StateView<Kind> source_state,
                                tyr::planning::StateView<Kind> target_state,
                                runir::kr::dl::semantics::Builder& dl_builder,
                                runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository,
                                runir::kr::dl::semantics::EvaluationWorkspace& workspace,
                                runir::kr::dl::semantics::DenotationCaches<runir::kr::ExtFamilyTag>& source_caches,
                                runir::kr::dl::semantics::DenotationCaches<runir::kr::ExtFamilyTag>& target_caches,
                                runir::kr::dl::semantics::CallArgumentsView arguments,
                                runir::kr::dl::semantics::RegisterValuesView source_registers,
                                runir::kr::dl::semantics::RegisterValuesView target_registers) noexcept :
        Base(DlContext(source_state, dl_builder, dl_denotation_repository, workspace, source_caches, arguments, source_registers),
             DlContext(std::move(target_state), dl_builder, dl_denotation_repository, workspace, target_caches, arguments, target_registers))
    {
    }
};

}  // namespace runir::kr::ps::dl

#endif
