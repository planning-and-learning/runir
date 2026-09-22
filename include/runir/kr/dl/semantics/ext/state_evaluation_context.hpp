#ifndef RUNIR_KR_DL_SEMANTICS_EXT_STATE_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_EXT_STATE_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/call_arguments_view.hpp"
#include "runir/kr/dl/semantics/register_values_view.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <utility>

namespace runir::kr::dl::semantics
{

template<tyr::TaskKind Kind>
class StateEvaluationContext<runir::kr::ExtFamilyTag, Kind> : public BaseStateEvaluationContext<runir::kr::ExtFamilyTag, Kind>
{
private:
    using Base = BaseStateEvaluationContext<runir::kr::ExtFamilyTag, Kind>;

    RegisterValuesView m_registers;
    CallArgumentsView m_arguments;

public:
    StateEvaluationContext(tyr::planning::StateView<Kind> state,
                           Builder& builder,
                           DenotationRepository& denotation_repository,
                           EvaluationWorkspace& workspace,
                           DenotationCaches<runir::kr::ExtFamilyTag>& caches,
                           CallArgumentsView arguments,
                           RegisterValuesView registers) noexcept :
        Base(std::move(state), builder, denotation_repository, workspace, caches),
        m_registers(registers),
        m_arguments(arguments)
    {
    }

    auto registers() const noexcept { return m_registers; }
    auto arguments() const noexcept { return m_arguments; }
};

}  // namespace runir::kr::dl::semantics

#endif
