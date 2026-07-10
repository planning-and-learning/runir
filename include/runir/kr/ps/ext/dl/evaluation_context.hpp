#ifndef RUNIR_KR_PS_EXT_DL_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_EXT_DL_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/semantics/evaluation_workspace.hpp"
#include "runir/kr/dl/semantics/ext/evaluation_context.hpp"
#include "runir/kr/ps/dl/evaluation_context.hpp"

#include <tyr/planning/declarations.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>

namespace runir::kr::ps::dl
{

template<tyr::planning::TaskKind Kind>
class EvaluationContext<runir::kr::ExtFamilyTag, Kind> :
    public BaseEvaluationContext<EvaluationContext<runir::kr::ExtFamilyTag, Kind>, runir::kr::ExtFamilyTag, Kind>
{
private:
    using Base = BaseEvaluationContext<EvaluationContext<runir::kr::ExtFamilyTag, Kind>, runir::kr::ExtFamilyTag, Kind>;

    runir::kr::dl::semantics::EvaluationWorkspace m_workspace;
    runir::kr::dl::semantics::EvaluationWorkspace* m_external_workspace;

public:
    using DlContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>;

    EvaluationContext(tyr::planning::StateView<Kind> source_state,
                      tyr::planning::StateView<Kind> target_state,
                      runir::kr::dl::semantics::Builder& dl_builder,
                      runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository,
                      runir::kr::dl::semantics::Arguments arguments = {},
                      runir::kr::dl::semantics::Registers registers = {}) noexcept :
        Base(DlContext(source_state, dl_builder, dl_denotation_repository, arguments, registers),
             DlContext(std::move(target_state), dl_builder, dl_denotation_repository, arguments, std::move(registers))),
        m_workspace(),
        m_external_workspace(nullptr)
    {
    }

    EvaluationContext(tyr::planning::StateView<Kind> source_state,
                      tyr::planning::StateView<Kind> target_state,
                      runir::kr::dl::semantics::Builder& dl_builder,
                      runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository,
                      runir::kr::dl::semantics::EvaluationWorkspace& workspace,
                      runir::kr::dl::semantics::Arguments arguments,
                      runir::kr::dl::semantics::Registers registers) noexcept :
        Base(DlContext(source_state,
                       dl_builder,
                       dl_denotation_repository,
                       arguments,
                       registers),
             DlContext(std::move(target_state),
                       dl_builder,
                       dl_denotation_repository,
                       arguments,
                       std::move(registers))),
        m_workspace(),
        m_external_workspace(&workspace)
    {
    }

    auto& get_workspace() noexcept { return m_external_workspace ? *m_external_workspace : m_workspace; }
};

}  // namespace runir::kr::ps::dl

#endif
