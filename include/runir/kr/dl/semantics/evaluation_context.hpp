#ifndef RUNIR_SEMANTICS_EVALUATION_CONTEXT_HPP_
#define RUNIR_SEMANTICS_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

template<tyr::planning::TaskKind Kind>
class EvaluationContext
{
private:
    tyr::planning::StateView<Kind> m_state;
    Builder& m_builder;
    DenotationRepository& m_denotation_repository;

public:
    EvaluationContext(tyr::planning::StateView<Kind> state, Builder& builder, DenotationRepository& denotation_repository) noexcept :
        m_state(std::move(state)),
        m_builder(builder),
        m_denotation_repository(denotation_repository)
    {
    }

    const auto& get_state() const noexcept { return m_state; }
    auto& get_builder() noexcept { return m_builder; }
    auto& get_denotation_repository() noexcept { return m_denotation_repository; }
    const auto& get_denotation_repository() const noexcept { return m_denotation_repository; }
};

}

#endif
