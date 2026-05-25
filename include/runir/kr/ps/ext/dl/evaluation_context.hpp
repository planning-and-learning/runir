#ifndef RUNIR_KR_PS_EXT_DL_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_PS_EXT_DL_EVALUATION_CONTEXT_HPP_

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

public:
    using DlContext = runir::kr::dl::semantics::EvaluationContext<runir::kr::ExtFamilyTag, Kind>;

    template<runir::kr::dl::CategoryTag Category>
    using Arguments = typename DlContext::template Arguments<Category>;

    EvaluationContext(tyr::planning::StateView<Kind> source_state,
                      tyr::planning::StateView<Kind> target_state,
                      runir::kr::dl::semantics::Builder& dl_builder,
                      runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository) noexcept :
        Base(std::move(source_state), std::move(target_state), dl_builder, dl_denotation_repository)
    {
    }

    EvaluationContext(tyr::planning::StateView<Kind> source_state,
                      tyr::planning::StateView<Kind> target_state,
                      runir::kr::dl::semantics::Builder& dl_builder,
                      runir::kr::dl::semantics::DenotationRepository& dl_denotation_repository,
                      Arguments<runir::kr::dl::ConceptTag> concept_arguments,
                      Arguments<runir::kr::dl::RoleTag> role_arguments,
                      Arguments<runir::kr::dl::BooleanTag> boolean_arguments,
                      Arguments<runir::kr::dl::NumericalTag> numerical_arguments) noexcept :
        Base(
            DlContext(source_state, dl_builder, dl_denotation_repository, concept_arguments, role_arguments, boolean_arguments, numerical_arguments),
            DlContext(std::move(target_state), dl_builder, dl_denotation_repository, concept_arguments, role_arguments, boolean_arguments, numerical_arguments))
    {
    }
};

}  // namespace runir::kr::ps::dl

#endif
