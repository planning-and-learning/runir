#include "runir/kr/dl/semantics/detail/evaluation_impl.hpp"

namespace runir::kr::dl::semantics
{

template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(FamilyQueryView<runir::kr::BaseFamilyTag>, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>&) -> QueryDenotationView;

template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, ConceptTag>, BaseStateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, RoleTag>, BaseStateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, BooleanTag>, BaseStateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, NumericalTag>, BaseStateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(FamilyQueryView<runir::kr::BaseFamilyTag>, BaseStateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>>&) -> QueryDenotationView;

}  // namespace runir::kr::dl::semantics
