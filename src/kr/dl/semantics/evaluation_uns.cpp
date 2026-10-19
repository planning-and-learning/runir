#include "runir/kr/dl/semantics/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/detail/repository_instantiations.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

namespace runir::kr::dl::semantics
{

template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(FamilyQueryView<runir::kr::UnsFamilyTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag>&) -> QueryDenotationView;

template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(FamilyQueryView<runir::kr::UnsFamilyTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&) -> QueryDenotationView;

template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, ConceptTag>, BaseStateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, RoleTag>, BaseStateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, BooleanTag>, BaseStateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, NumericalTag>, BaseStateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(FamilyQueryView<runir::kr::UnsFamilyTag>, BaseStateEvaluationContext<runir::kr::UnsFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>&) -> QueryDenotationView;

template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(FamilyQueryView<runir::kr::UnsFamilyTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>&) -> QueryDenotationView;

template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(FamilyQueryView<runir::kr::UnsFamilyTag>, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&) -> QueryDenotationView;

template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, ConceptTag>, BaseStateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, RoleTag>, BaseStateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, BooleanTag>, BaseStateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, NumericalTag>, BaseStateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(FamilyQueryView<runir::kr::UnsFamilyTag>, BaseStateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>>&) -> QueryDenotationView;

}  // namespace runir::kr::dl::semantics

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

template class EvaluationGraph<runir::kr::UnsFamilyTag, tyr::LiftedTag>;
template class IncrementalEvaluation<runir::kr::UnsFamilyTag, tyr::LiftedTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::UnsFamilyTag, tyr::LiftedTag>::prepare<ConceptTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::UnsFamilyTag, tyr::LiftedTag>::prepare<RoleTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::UnsFamilyTag, tyr::LiftedTag>::prepare<BooleanTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::UnsFamilyTag, tyr::LiftedTag>::prepare<NumericalTag>(FamilyConstructorView<runir::kr::UnsFamilyTag, NumericalTag>);

template void IncrementalEvaluation<runir::kr::UnsFamilyTag, tyr::LiftedTag>::demand<StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>>(ygg::uint_t, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag>&, const Delta<runir::kr::UnsFamilyTag>*, ygg::database::Workspace<QueryValues>*);
template void IncrementalEvaluation<runir::kr::UnsFamilyTag, tyr::LiftedTag>::demand<StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>>(ygg::uint_t, StateEvaluationContext<runir::kr::UnsFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&, const Delta<runir::kr::UnsFamilyTag>*, ygg::database::Workspace<QueryValues>*);

}  // namespace runir::kr::dl::semantics::incremental
