#include "runir/kr/dl/semantics/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/detail/repository_instantiations.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

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

namespace runir::kr::dl::semantics::incremental
{

template class EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>;
template class IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::GroundTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::prepare<ConceptTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::prepare<RoleTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::prepare<BooleanTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::prepare<NumericalTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, NumericalTag>);

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::get_index<ConceptTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, ConceptTag>) const;
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::get_index<RoleTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, RoleTag>) const;
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::get_index<BooleanTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, BooleanTag>) const;
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::get_index<NumericalTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, NumericalTag>) const;
template BorrowedDenotationView<ConceptTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::result<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template BorrowedDenotationView<RoleTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::result<RoleTag>(EvaluationIndex<RoleTag>) const;
template BorrowedDenotationView<BooleanTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::result<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template BorrowedDenotationView<NumericalTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::result<NumericalTag>(EvaluationIndex<NumericalTag>) const;
template const DenotationDelta<ConceptTag>& EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::change<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template const DenotationDelta<RoleTag>& EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::change<RoleTag>(EvaluationIndex<RoleTag>) const;
template size_t EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::cardinality<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template size_t EvaluationGraph<runir::kr::BaseFamilyTag, tyr::GroundTag>::cardinality<RoleTag>(EvaluationIndex<RoleTag>) const;
template bool IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::GroundTag>::changed<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template bool IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::GroundTag>::changed<NumericalTag>(EvaluationIndex<NumericalTag>) const;

template void IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::GroundTag>::demand<StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>>(ygg::uint_t, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::GroundTag>&, const Delta<runir::kr::BaseFamilyTag>*, ygg::database::Workspace*);

template class EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>;
template class IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::LiftedTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::prepare<ConceptTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::prepare<RoleTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::prepare<BooleanTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::prepare<NumericalTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, NumericalTag>);

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::get_index<ConceptTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, ConceptTag>) const;
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::get_index<RoleTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, RoleTag>) const;
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::get_index<BooleanTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, BooleanTag>) const;
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::get_index<NumericalTag>(FamilyConstructorView<runir::kr::BaseFamilyTag, NumericalTag>) const;
template BorrowedDenotationView<ConceptTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::result<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template BorrowedDenotationView<RoleTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::result<RoleTag>(EvaluationIndex<RoleTag>) const;
template BorrowedDenotationView<BooleanTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::result<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template BorrowedDenotationView<NumericalTag> EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::result<NumericalTag>(EvaluationIndex<NumericalTag>) const;
template const DenotationDelta<ConceptTag>& EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::change<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template const DenotationDelta<RoleTag>& EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::change<RoleTag>(EvaluationIndex<RoleTag>) const;
template size_t EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::cardinality<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template size_t EvaluationGraph<runir::kr::BaseFamilyTag, tyr::LiftedTag>::cardinality<RoleTag>(EvaluationIndex<RoleTag>) const;
template bool IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::LiftedTag>::changed<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template bool IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::LiftedTag>::changed<NumericalTag>(EvaluationIndex<NumericalTag>) const;

template void IncrementalEvaluation<runir::kr::BaseFamilyTag, tyr::LiftedTag>::demand<StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>>(ygg::uint_t, StateEvaluationContext<runir::kr::BaseFamilyTag, tyr::LiftedTag>&, const Delta<runir::kr::BaseFamilyTag>*, ygg::database::Workspace*);

}  // namespace runir::kr::dl::semantics::incremental
