#include "runir/kr/dl/semantics/detail/evaluation_impl.hpp"

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
