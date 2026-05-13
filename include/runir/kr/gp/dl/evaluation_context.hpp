#ifndef RUNIR_KR_GP_DL_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_GP_DL_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/evaluation_context.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::gp::dl
{

template<tyr::planning::TaskKind Kind>
class EvaluationContext
{
private:
    runir::kr::dl::semantics::EvaluationContext<Kind> m_source_context;
    runir::kr::dl::semantics::EvaluationContext<Kind> m_target_context;

public:
    EvaluationContext(tyr::planning::StateView<Kind> source_state,
                      tyr::planning::StateView<Kind> target_state,
                      runir::kr::dl::semantics::Builder& dl_builder,
                      runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository) noexcept :
        m_source_context(std::move(source_state), dl_builder, dl_denotation_repository),
        m_target_context(std::move(target_state), dl_builder, dl_denotation_repository)
    {
    }

    const auto& get_source_state() const noexcept { return m_source_context.get_state(); }
    const auto& get_target_state() const noexcept { return m_target_context.get_state(); }
    auto& get_source_context() noexcept { return m_source_context; }
    auto& get_target_context() noexcept { return m_target_context; }
};

}  // namespace runir::kr::gp::dl

#endif
