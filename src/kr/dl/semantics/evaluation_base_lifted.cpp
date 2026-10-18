#include "runir/kr/dl/semantics/detail/evaluation_impl.hpp"

namespace runir::kr::dl::semantics
{

template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(FamilyQueryView<runir::kr::BaseFamilyTag>, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>&) -> QueryDenotationView;

template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, ConceptTag>, BaseStateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, RoleTag>, BaseStateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, BooleanTag>, BaseStateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, NumericalTag>, BaseStateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(FamilyQueryView<runir::kr::BaseFamilyTag>, BaseStateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>>&) -> QueryDenotationView;

}  // namespace runir::kr::dl::semantics
