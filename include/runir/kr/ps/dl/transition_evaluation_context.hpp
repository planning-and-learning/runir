#ifndef RUNIR_KR_PS_DL_TRANSITION_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_DL_TRANSITION_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/ext/state_evaluation_context.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <utility>

namespace runir::kr::ps::dl
{

template<runir::kr::FamilyTag Family, tyr::TaskKind Kind>
class TransitionEvaluationContext
{
public:
    using DlFamily = typename PsFamilyTraits<Family>::DlFamily;
    using DlContext = runir::kr::dl::semantics::StateEvaluationContext<DlFamily, Kind>;

private:
    DlContext m_source_context;
    DlContext m_target_context;

public:
    TransitionEvaluationContext(tyr::planning::StateView<Kind> source_state,
                                tyr::planning::StateView<Kind> target_state,
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

    TransitionEvaluationContext(tyr::planning::StateView<Kind> source_state,
                                tyr::planning::StateView<Kind> target_state,
                                runir::kr::dl::semantics::Builder& dl_builder,
                                runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository,
                                runir::kr::dl::semantics::EvaluationWorkspace& workspace,
                                runir::kr::dl::semantics::DenotationCaches<DlFamily>& source_caches,
                                runir::kr::dl::semantics::DenotationCaches<DlFamily>& target_caches,
                                runir::kr::dl::semantics::CallArgumentsView arguments,
                                runir::kr::dl::semantics::RegisterValuesView source_registers,
                                runir::kr::dl::semantics::RegisterValuesView target_registers) noexcept
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
