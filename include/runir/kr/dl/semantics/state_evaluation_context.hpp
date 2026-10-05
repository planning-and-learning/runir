#ifndef RUNIR_KR_DL_SEMANTICS_STATE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_STATE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_storage.hpp"
#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"

#include <array>
#include <concepts>
#include <stdexcept>
#include <type_traits>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

/// Evaluation reads state contents; it does not require registered state identity.
template<typename Context, typename Family, typename Kind>
concept StateEvaluationResourcesConcept = FamilyTag<Family> && tyr::TaskKind<Kind> && requires(Context& context, const Context& const_context) {
    { const_context.get_state() } -> tyr::planning::StateViewConcept<Kind>;
    { context.get_builder() } -> std::same_as<Builder&>;
    { context.get_denotation_repository() } -> std::same_as<DenotationRepository&>;
    { context.get_workspace() } -> std::same_as<EvaluationWorkspace&>;
    { context.get_caches() } -> std::same_as<DenotationCaches<Family>&>;
} && (!std::same_as<Family, runir::kr::ExtFamilyTag> || requires(const Context& context) {
                                              { context.arguments() } -> std::same_as<CallArgumentsView>;
                                              { context.registers() } -> RegisterValuesViewConcept;
                                          });

/// Recursive contexts retain their family, task kind, resources, and navigation.
template<typename Context, typename Family, typename Kind>
concept StateEvaluationNavigationConcept = StateEvaluationResourcesConcept<Context, Family, Kind> && requires(const Context& context) {
    { context.for_result(false) } -> std::same_as<Context>;
    { context.child_context() } -> std::same_as<Context>;
};

template<typename Context, typename Family, typename Kind>
concept StateEvaluationContextConcept = StateEvaluationResourcesConcept<std::remove_reference_t<Context>, Family, Kind>
                                       && requires(const std::remove_reference_t<Context>& context) {
                                              { context.for_result(false) } -> StateEvaluationNavigationConcept<Family, Kind>;
                                              { context.child_context() } -> StateEvaluationNavigationConcept<Family, Kind>;
                                          };

template<FamilyTag Family, tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> S = tyr::planning::StateView<Kind>>
class BaseStateEvaluationContext
{
private:
    S m_state;
    Builder& m_builder;
    DenotationCaches<Family>* m_caches;
    std::array<DenotationRepository*, 2> m_denotations;
    EvaluationStorage<Family>& m_intermediates;
    bool m_result_is_static = false;

protected:
    void select_result(bool is_static) noexcept { m_result_is_static = is_static; }
    void select_children() noexcept
    {
        m_caches = &m_intermediates.get_caches();
        m_denotations = { &m_intermediates.get_denotation_repository(false), &m_intermediates.get_denotation_repository(true) };
    }

public:
    using FamilyType = Family;
    using KindType = Kind;

    BaseStateEvaluationContext(S state, Builder& builder, EvaluationStorage<Family>& storage) :
        m_state(std::move(state)),
        m_builder(builder),
        m_caches(&storage.get_caches()),
        m_denotations { &storage.get_denotation_repository(false), &storage.get_denotation_repository(true) },
        m_intermediates(storage)
    {
        if (storage.get_denotation_repository(false).get_formalism_repository_ptr() != m_state.get_repository())
            throw std::invalid_argument("Evaluation requires a result repository for the same planning task.");
    }

    /// Retain computed roots in an existing repository while children use reusable storage.
    BaseStateEvaluationContext(S state,
                               Builder& builder,
                               DenotationCaches<Family>& caches,
                               DenotationRepository& repository,
                               EvaluationStorage<Family>& intermediates) :
        BaseStateEvaluationContext(std::move(state), builder, intermediates)
    {
        if (repository.get_formalism_repository_ptr() != m_state.get_repository())
            throw std::invalid_argument("Evaluation requires a result repository for the same planning task.");
        m_caches = &caches;
        m_denotations = { &repository, &repository };
    }

    const auto& get_state() const noexcept { return m_state; }
    auto& get_builder() noexcept { return m_builder; }
    auto& get_denotation_repository(bool is_static) const noexcept { return *m_denotations[is_static]; }
    auto& get_denotation_repository() const noexcept { return get_denotation_repository(m_result_is_static); }
    auto& get_workspace() noexcept { return m_builder.get_workspace(); }
    auto& get_caches() noexcept { return *m_caches; }

    /// Copies only borrowed views and references; result payloads stay put.
    auto for_result(bool is_static) const noexcept
    {
        auto result = *this;
        result.select_result(is_static);
        return result;
    }

    /// Recursive results use the reusable intermediate caches and repositories.
    auto child_context() const noexcept
    {
        auto result = *this;
        result.select_children();
        return result;
    }
};

template<FamilyTag Family,
         tyr::TaskKind Kind,
         tyr::planning::StateViewConcept<Kind> S = tyr::planning::StateView<Kind>,
         RegisterValuesViewConcept R = RegisterValuesView>
class StateEvaluationContext;

template<FamilyTag Family, tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> S, RegisterValuesViewConcept R>
    requires(!std::same_as<Family, runir::kr::ExtFamilyTag>)
class StateEvaluationContext<Family, Kind, S, R> : public BaseStateEvaluationContext<Family, Kind, S>
{
public:
    using BaseStateEvaluationContext<Family, Kind, S>::BaseStateEvaluationContext;
};

template<FamilyTag Family, tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> State>
const auto& get_repository(const BaseStateEvaluationContext<Family, Kind, State>& context) noexcept
{
    return context.get_state().get_repository();
}

}

#endif
