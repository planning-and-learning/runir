#ifndef RUNIR_KR_PS_BASE_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_BASE_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/ps/base/dl/transition_evaluation_context.hpp"
#include "runir/kr/task_context.hpp"

#include <utility>

namespace runir::kr::ps::base
{

template<tyr::TaskKind Kind>
class EvaluationEnvironment
{
private:
    using StateDlContext = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::BaseFamilyTag, Kind>;
    using TransitionDlContext = runir::kr::ps::dl::TransitionEvaluationContext<runir::kr::BaseFamilyTag, Kind>;

    runir::kr::dl::semantics::Builder& m_dl_builder;
    runir::kr::dl::semantics::DenotationRepository& m_dl_denotation_repository;
    runir::kr::dl::semantics::DenotationCaches<runir::kr::BaseFamilyTag> m_dl_caches;
    runir::kr::dl::semantics::DenotationCaches<runir::kr::BaseFamilyTag> m_dl_target_caches;

public:
    explicit EvaluationEnvironment(runir::kr::TaskContext<Kind>& task_context) :
        m_dl_builder(task_context.dl_builder),
        m_dl_denotation_repository(*task_context.dl_denotation_repository)
    {
    }

    auto& get_dl_workspace() noexcept { return m_dl_builder.get_workspace(); }
    auto& get_dl_caches() noexcept { return m_dl_caches; }
    auto& get_dl_target_caches() noexcept { return m_dl_target_caches; }

    StateDlContext make_dl_context(tyr::planning::StateView<Kind> state)
    {
        return StateDlContext(std::move(state), m_dl_builder, m_dl_denotation_repository, get_dl_workspace(), m_dl_caches);
    }

    TransitionDlContext make_dl_transition_context(tyr::planning::StateView<Kind> source_state, tyr::planning::StateView<Kind> target_state)
    {
        return TransitionDlContext(std::move(source_state),
                                   std::move(target_state),
                                   m_dl_builder,
                                   m_dl_denotation_repository,
                                   get_dl_workspace(),
                                   m_dl_caches,
                                   m_dl_target_caches);
    }
};

}  // namespace runir::kr::ps::base

#endif
