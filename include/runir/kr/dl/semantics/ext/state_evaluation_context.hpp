#ifndef RUNIR_KR_DL_SEMANTICS_EXT_STATE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_EXT_STATE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <stdexcept>
#include <utility>

namespace runir::kr::dl::semantics
{

template<tyr::TaskKind Kind, tyr::planning::StateViewConcept<Kind> S, RegisterValuesViewConcept R>
class StateEvaluationContext<runir::kr::ExtFamilyTag, Kind, S, R> : public BaseStateEvaluationContext<runir::kr::ExtFamilyTag, Kind, S>
{
private:
    using Base = BaseStateEvaluationContext<runir::kr::ExtFamilyTag, Kind, S>;

    R m_registers;
    CallArgumentsView m_arguments;

    void validate_inputs() const
    {
        const auto* repository = this->get_state().get_formalism_repository().get();
        if (&m_arguments.get_context().get_formalism_repository() != repository)
            throw std::invalid_argument("Evaluation requires arguments for the same planning task.");
        if (&m_registers.get_formalism_repository() != repository)
            throw std::invalid_argument("Evaluation requires registers for the same planning task.");
    }

public:
    StateEvaluationContext(S state, Builder& builder, EvaluationStorage<runir::kr::ExtFamilyTag>& storage, CallArgumentsView arguments, R registers) :
        Base(std::move(state), builder, storage),
        m_registers(registers),
        m_arguments(arguments)
    {
        validate_inputs();
    }

    StateEvaluationContext(S state,
                           Builder& builder,
                           DenotationCaches<runir::kr::ExtFamilyTag>& caches,
                           DenotationRepository& repository,
                           EvaluationStorage<runir::kr::ExtFamilyTag>& intermediates,
                           CallArgumentsView arguments,
                           R registers) :
        Base(std::move(state), builder, caches, repository, intermediates),
        m_registers(registers),
        m_arguments(arguments)
    {
        validate_inputs();
    }

    auto for_result(bool is_static) const noexcept
    {
        auto result = *this;
        result.select_result(is_static);
        return result;
    }

    auto child_context() const noexcept
    {
        auto result = *this;
        result.select_children();
        return result;
    }

    auto registers() const noexcept { return m_registers; }
    auto arguments() const noexcept { return m_arguments; }
};

}  // namespace runir::kr::dl::semantics

#endif
