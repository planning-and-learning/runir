#ifndef RUNIR_SEMANTICS_EVALUATION_CONTEXT_HPP_
#define RUNIR_SEMANTICS_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/declarations.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

template<typename Derived, tyr::planning::TaskKind Kind>
class BaseEvaluationContext
{
private:
    tyr::planning::StateView<Kind> m_state;
    Builder& m_builder;
    DenotationRepository& m_denotation_repository;

protected:
    BaseEvaluationContext(tyr::planning::StateView<Kind> state, Builder& builder, DenotationRepository& denotation_repository) noexcept :
        m_state(std::move(state)),
        m_builder(builder),
        m_denotation_repository(denotation_repository)
    {
    }

public:
    const auto& get_state() const noexcept { return m_state; }
    auto& get_builder() noexcept { return m_builder; }
    auto& get_denotation_repository() noexcept { return m_denotation_repository; }
    const auto& get_denotation_repository() const noexcept { return m_denotation_repository; }
};

template<runir::kr::dl::FamilyTag Family, tyr::planning::TaskKind Kind>
class EvaluationContext;

template<runir::kr::dl::FamilyTag Family, tyr::planning::TaskKind Kind>
const auto& get_repository(const EvaluationContext<Family, Kind>& context) noexcept
{
    return context.get_state().get_state_repository()->get_task()->get_repository();
}

}

#endif
