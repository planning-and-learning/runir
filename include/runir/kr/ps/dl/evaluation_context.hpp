#ifndef RUNIR_KR_PS_DL_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_DL_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_context.hpp"
#include "runir/kr/ps/dl/declarations.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::ps::dl
{

template<typename Derived, runir::kr::FamilyTag Family_, tyr::planning::TaskKind Kind>
class BaseEvaluationContext
{
private:
    runir::kr::dl::semantics::EvaluationContext<Family_, Kind> m_source_context;
    runir::kr::dl::semantics::EvaluationContext<Family_, Kind> m_target_context;

protected:
    BaseEvaluationContext(tyr::planning::StateView<Kind> source_state,
                          tyr::planning::StateView<Kind> target_state,
                          runir::kr::dl::semantics::Builder& dl_builder,
                          runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository) noexcept :
        m_source_context(std::move(source_state), dl_builder, dl_denotation_repository),
        m_target_context(std::move(target_state), dl_builder, dl_denotation_repository)
    {
    }

public:
    using Family = Family_;

    const auto& get_source_state() const noexcept { return m_source_context.get_state(); }
    const auto& get_target_state() const noexcept { return m_target_context.get_state(); }
    auto& get_source_context() noexcept { return m_source_context; }
    auto& get_target_context() noexcept { return m_target_context; }
};

template<runir::kr::FamilyTag Family, tyr::planning::TaskKind Kind>
class EvaluationContext;

}  // namespace runir::kr::ps::dl

#endif
