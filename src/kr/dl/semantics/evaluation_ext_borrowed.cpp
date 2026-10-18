#include "runir/kr/dl/semantics/detail/evaluation_impl.hpp"

namespace runir::kr::dl::semantics
{

template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, BorrowedRegisterValuesView>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, BorrowedRegisterValuesView>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, BorrowedRegisterValuesView>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, BorrowedRegisterValuesView>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(FamilyQueryView<runir::kr::ExtFamilyTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, BorrowedRegisterValuesView>&) -> QueryDenotationView;

template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(FamilyQueryView<runir::kr::ExtFamilyTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&) -> QueryDenotationView;

}  // namespace runir::kr::dl::semantics
