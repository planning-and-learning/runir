#ifndef RUNIR_KR_PS_DL_TRANSITION_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_DL_TRANSITION_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/ext/state_evaluation_context.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <type_traits>
#include <utility>

namespace runir::kr::ps::dl
{

template<typename Context, typename Family = typename Context::FamilyType>
concept TransitionEvaluationContextConcept = runir::kr::FamilyTag<Family> && requires(Context& context) {
    requires std::same_as<typename Context::FamilyType, Family>;
    requires runir::kr::dl::semantics::StateEvaluationContextConcept<std::remove_cvref_t<decltype(context.get_source_context())>,
                                                                     typename PsFamilyTraits<Family>::DlFamily>;
    requires runir::kr::dl::semantics::StateEvaluationContextConcept<std::remove_cvref_t<decltype(context.get_target_context())>,
                                                                     typename PsFamilyTraits<Family>::DlFamily>;
    requires std::same_as<typename std::remove_cvref_t<decltype(context.get_source_context())>::KindType,
                          typename std::remove_cvref_t<decltype(context.get_target_context())>::KindType>;
};

template<runir::kr::FamilyTag Family, tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
class TransitionEvaluationContext
{
public:
    using FamilyType = Family;
    using KindType = Kind;
    using DlFamily = typename PsFamilyTraits<Family>::DlFamily;
    using DlContext = runir::kr::dl::semantics::StateEvaluationContext<DlFamily, Kind, S, R>;

private:
    DlContext m_source_context;
    DlContext m_target_context;

public:
    TransitionEvaluationContext(S source_state,
                                S target_state,
                                runir::kr::dl::semantics::Builder& dl_builder,
                                runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository,
                                runir::kr::dl::semantics::EvaluationWorkspace& workspace,
                                runir::kr::dl::semantics::DenotationCaches<DlFamily>& source_caches,
                                runir::kr::dl::semantics::DenotationCaches<DlFamily>& target_caches) noexcept
        requires(!std::same_as<DlFamily, runir::kr::ExtFamilyTag>)
        :
        m_source_context(std::move(source_state), dl_builder, dl_denotation_repository, workspace, source_caches),
        m_target_context(std::move(target_state), dl_builder, dl_denotation_repository, workspace, target_caches)
    {
    }

    TransitionEvaluationContext(S source_state,
                                S target_state,
                                runir::kr::dl::semantics::Builder& dl_builder,
                                runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository,
                                runir::kr::dl::semantics::EvaluationWorkspace& workspace,
                                runir::kr::dl::semantics::DenotationCaches<DlFamily>& source_caches,
                                runir::kr::dl::semantics::DenotationCaches<DlFamily>& target_caches,
                                runir::kr::dl::semantics::CallArgumentsView arguments,
                                R source_registers,
                                R target_registers) noexcept
        requires std::same_as<DlFamily, runir::kr::ExtFamilyTag>
        :
        m_source_context(std::move(source_state), dl_builder, dl_denotation_repository, workspace, source_caches, arguments, source_registers),
        m_target_context(std::move(target_state), dl_builder, dl_denotation_repository, workspace, target_caches, arguments, target_registers)
    {
    }

    const auto& get_source_state() const noexcept { return m_source_context.get_state(); }
    const auto& get_target_state() const noexcept { return m_target_context.get_state(); }
    auto& get_source_context() noexcept { return m_source_context; }
    auto& get_target_context() noexcept { return m_target_context; }
};

}  // namespace runir::kr::ps::dl

#endif
