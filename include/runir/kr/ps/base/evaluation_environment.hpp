#ifndef RUNIR_KR_PS_BASE_EVALUATION_ENVIRONMENT_HPP_
#define RUNIR_KR_PS_BASE_EVALUATION_ENVIRONMENT_HPP_

#include "runir/kr/dl/semantics/evaluation_storage.hpp"
#include "runir/kr/ps/dl/transition_evaluation_context.hpp"
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
    runir::kr::dl::semantics::EvaluationStorage<runir::kr::BaseFamilyTag> m_source_storage;
    runir::kr::dl::semantics::EvaluationStorage<runir::kr::BaseFamilyTag> m_target_storage;

public:
    explicit EvaluationEnvironment(runir::kr::TaskContext<Kind>& task_context) :
        m_dl_builder(task_context.dl_builder),
        m_source_storage(*task_context.dl_denotation_repository),
        m_target_storage(*task_context.dl_denotation_repository)
    {
    }

    auto& get_dl_caches() noexcept { return m_source_storage.get_caches(); }
    auto& get_dl_target_caches() noexcept { return m_target_storage.get_caches(); }
    void reset_source() noexcept { m_source_storage.reset_dynamic(); }
    void reset_target() noexcept { m_target_storage.reset_dynamic(); }

    StateDlContext make_dl_context(tyr::planning::StateView<Kind> state) { return StateDlContext(std::move(state), m_dl_builder, m_source_storage); }

    TransitionDlContext make_dl_transition_context(tyr::planning::StateView<Kind> source_state, tyr::planning::StateView<Kind> target_state)
    {
        return TransitionDlContext(std::move(source_state), std::move(target_state), m_dl_builder, m_source_storage, m_target_storage);
    }
};

}  // namespace runir::kr::ps::base

#endif
