#include "runir/kr/dl/semantics/detail/evaluation_impl.hpp"

namespace runir::kr::dl::semantics
{

template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(FamilyQueryView<runir::kr::ExtFamilyTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag>&) -> QueryDenotationView;

template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(FamilyQueryView<runir::kr::ExtFamilyTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag>&) -> QueryDenotationView;

}  // namespace runir::kr::dl::semantics
