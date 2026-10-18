#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"

namespace runir::kr::dl::semantics::incremental
{

template class EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>;
template class IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::LiftedTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::prepare<ConceptTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::prepare<RoleTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::prepare<BooleanTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::prepare<NumericalTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>);

template void IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::LiftedTag>::demand<StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag>>(ygg::uint_t, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag>&, const Delta<runir::kr::ExtFamilyTag>*, ygg::database::Workspace<QueryValues>*);
template void IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::LiftedTag>::demand<StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>, BorrowedRegisterValuesView>>(ygg::uint_t, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&, const Delta<runir::kr::ExtFamilyTag>*, ygg::database::Workspace<QueryValues>*);

}  // namespace runir::kr::dl::semantics::incremental
