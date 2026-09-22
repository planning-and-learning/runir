#ifndef RUNIR_KR_PS_DL_TRANSITION_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_DL_TRANSITION_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::ps::dl
{

template<runir::kr::FamilyTag Family, tyr::TaskKind Kind>
class BaseTransitionEvaluationContext
{
private:
    runir::kr::dl::semantics::StateEvaluationContext<Family, Kind> m_source_context;
    runir::kr::dl::semantics::StateEvaluationContext<Family, Kind> m_target_context;

protected:
    BaseTransitionEvaluationContext(tyr::planning::StateView<Kind> source_state,
                                    tyr::planning::StateView<Kind> target_state,
                                    runir::kr::dl::semantics::Builder& dl_builder,
                                    runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository,
                                    runir::kr::dl::semantics::EvaluationWorkspace& workspace,
                                    runir::kr::dl::semantics::DenotationCaches<Family>& source_caches,
                                    runir::kr::dl::semantics::DenotationCaches<Family>& target_caches) noexcept :
        m_source_context(std::move(source_state), dl_builder, dl_denotation_repository, workspace, source_caches),
        m_target_context(std::move(target_state), dl_builder, dl_denotation_repository, workspace, target_caches)
    {
    }

    BaseTransitionEvaluationContext(runir::kr::dl::semantics::StateEvaluationContext<Family, Kind> source_context,
                                    runir::kr::dl::semantics::StateEvaluationContext<Family, Kind> target_context) noexcept :
        m_source_context(std::move(source_context)),
        m_target_context(std::move(target_context))
    {
    }

public:
    const auto& get_source_state() const noexcept { return m_source_context.get_state(); }
    const auto& get_target_state() const noexcept { return m_target_context.get_state(); }
    auto& get_source_context() noexcept { return m_source_context; }
    auto& get_target_context() noexcept { return m_target_context; }
};

template<runir::kr::FamilyTag Family, tyr::TaskKind Kind>
class TransitionEvaluationContext;

}  // namespace runir::kr::ps::dl

#endif
