#ifndef RUNIR_KR_DL_SEMANTICS_STATE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_STATE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

template<FamilyTag Family, tyr::TaskKind Kind>
class BaseStateEvaluationContext
{
private:
    tyr::planning::StateView<Kind> m_state;
    Builder& m_builder;
    DenotationRepository& m_denotation_repository;
    EvaluationWorkspace& m_workspace;
    DenotationCaches<Family>& m_caches;

protected:
    BaseStateEvaluationContext(tyr::planning::StateView<Kind> state,
                               Builder& builder,
                               DenotationRepository& denotation_repository,
                               EvaluationWorkspace& workspace,
                               DenotationCaches<Family>& caches) noexcept :
        m_state(std::move(state)),
        m_builder(builder),
        m_denotation_repository(denotation_repository),
        m_workspace(workspace),
        m_caches(caches)
    {
    }

public:
    const auto& get_state() const noexcept { return m_state; }
    auto& get_builder() noexcept { return m_builder; }
    auto& get_denotation_repository() noexcept { return m_denotation_repository; }
    const auto& get_denotation_repository() const noexcept { return m_denotation_repository; }
    auto& get_workspace() noexcept { return m_workspace; }
    auto& get_caches() noexcept { return m_caches; }
};

template<runir::kr::dl::FamilyTag Family, tyr::TaskKind Kind>
class StateEvaluationContext;

template<runir::kr::dl::FamilyTag Family, tyr::TaskKind Kind>
const auto& get_repository(const StateEvaluationContext<Family, Kind>& context) noexcept
{
    return context.get_state().get_state_repository()->get_task()->get_repository();
}

}

#endif
