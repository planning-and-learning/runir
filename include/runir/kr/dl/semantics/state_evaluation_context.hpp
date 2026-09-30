#ifndef RUNIR_KR_DL_SEMANTICS_STATE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_STATE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"

#include <concepts>
#include <type_traits>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

/// Evaluation reads state contents; it does not require registered state identity.
template<typename Context, typename Family = typename Context::FamilyType>
concept StateEvaluationContextConcept = FamilyTag<Family> && requires(Context& context, const Context& const_context) {
    typename Context::KindType;
    requires tyr::TaskKind<typename Context::KindType>;
    requires std::same_as<typename Context::FamilyType, Family>;
    requires tyr::planning::StateViewConcept<std::remove_cvref_t<decltype(const_context.get_state())>, typename Context::KindType>;
    { context.get_builder() } -> std::same_as<Builder&>;
    { context.get_denotation_repository() } -> std::same_as<DenotationRepository&>;
    { context.get_workspace() } -> std::same_as<EvaluationWorkspace&>;
    { context.get_caches() } -> std::same_as<DenotationCaches<Family>&>;
};

template<FamilyTag Family, tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> S = tyr::planning::StateView<Kind>>
class BaseStateEvaluationContext
{
private:
    S m_state;
    Builder& m_builder;
    DenotationRepository& m_denotation_repository;
    EvaluationWorkspace& m_workspace;
    DenotationCaches<Family>& m_caches;

public:
    using FamilyType = Family;
    using KindType = Kind;
    using StateType = S;

    BaseStateEvaluationContext(S state,
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
    requires(!std::same_as<Family, runir::kr::ExtFamilyTag>)
class StateEvaluationContext<Family, Kind> : public BaseStateEvaluationContext<Family, Kind>
{
public:
    using BaseStateEvaluationContext<Family, Kind>::BaseStateEvaluationContext;
};

template<StateEvaluationContextConcept Context>
const auto& get_repository(const Context& context) noexcept
{
    return context.get_state().get_repository();
}

}

#endif
