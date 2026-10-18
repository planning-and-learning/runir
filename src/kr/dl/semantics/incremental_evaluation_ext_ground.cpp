#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"

namespace runir::kr::dl::semantics::incremental
{

template class EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>;
template class IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::GroundTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::prepare<ConceptTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::prepare<RoleTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::prepare<BooleanTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::prepare<NumericalTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>);

template void IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::GroundTag>::demand<StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag>>(ygg::uint_t, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag>&, const Delta<runir::kr::ExtFamilyTag>*, ygg::database::Workspace<QueryValues>*);
template void IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::GroundTag>::demand<StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>, BorrowedRegisterValuesView>>(ygg::uint_t, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>, BorrowedRegisterValuesView>&, const Delta<runir::kr::ExtFamilyTag>*, ygg::database::Workspace<QueryValues>*);
template void IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::GroundTag>::demand<StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, BorrowedRegisterValuesView>>(ygg::uint_t, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, BorrowedRegisterValuesView>&, const Delta<runir::kr::ExtFamilyTag>*, ygg::database::Workspace<QueryValues>*);

}  // namespace runir::kr::dl::semantics::incremental
