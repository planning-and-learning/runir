#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

namespace runir::kr::dl::semantics::incremental
{

template class EvaluationGraph<runir::kr::UnsFamilyTag, tyr::GroundTag>;
template class IncrementalEvaluation<runir::kr::UnsFamilyTag, tyr::GroundTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::UnsFamilyTag, tyr::GroundTag>::prepare<ConceptTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::UnsFamilyTag, tyr::GroundTag>::prepare<RoleTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::UnsFamilyTag, tyr::GroundTag>::prepare<BooleanTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::UnsFamilyTag, tyr::GroundTag>::prepare<NumericalTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, NumericalTag>);

template void IncrementalEvaluation<runir::kr::UnsFamilyTag, tyr::GroundTag>::demand<StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>>(ygg::uint_t, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>&, const Delta<runir::kr::UnsFamilyTag>*, ygg::database::Workspace<QueryValues>*);
template void IncrementalEvaluation<runir::kr::UnsFamilyTag, tyr::GroundTag>::demand<StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>>(ygg::uint_t, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&, const Delta<runir::kr::UnsFamilyTag>*, ygg::database::Workspace<QueryValues>*);

}  // namespace runir::kr::dl::semantics::incremental
