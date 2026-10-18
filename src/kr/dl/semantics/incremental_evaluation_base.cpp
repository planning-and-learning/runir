#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

namespace runir::kr::dl::semantics::incremental
{

template class EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>;
template class IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::GroundTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::prepare<ConceptTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::prepare<RoleTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::prepare<BooleanTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::prepare<NumericalTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, NumericalTag>);

template void IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::GroundTag>::demand<StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>>(ygg::uint_t, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>&, const Delta<runir::kr::BaseFamilyTag>*, ygg::database::Workspace<QueryValues>*);

template class EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>;
template class IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::LiftedTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::prepare<ConceptTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::prepare<RoleTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::prepare<BooleanTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::prepare<NumericalTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, NumericalTag>);

template void IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::LiftedTag>::demand<StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>>(ygg::uint_t, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>&, const Delta<runir::kr::BaseFamilyTag>*, ygg::database::Workspace<QueryValues>*);

}  // namespace runir::kr::dl::semantics::incremental
