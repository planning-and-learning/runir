#include "runir/kr/dl/semantics/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/detail/repository_instantiations.hpp"
#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"

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

template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>, BorrowedRegisterValuesView>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>, BorrowedRegisterValuesView>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>, BorrowedRegisterValuesView>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>, BorrowedRegisterValuesView>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(FamilyQueryView<runir::kr::ExtFamilyTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>, BorrowedRegisterValuesView>&) -> QueryDenotationView;

template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(FamilyQueryView<runir::kr::ExtFamilyTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&) -> QueryDenotationView;

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

/// Queries over builder states with interned register values, which the successor expander tests evaluate.
template auto evaluate<tyr::GroundTag>(FamilyQueryView<runir::kr::ExtFamilyTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&) -> QueryDenotationView;
template auto evaluate<tyr::LiftedTag>(FamilyQueryView<runir::kr::ExtFamilyTag>, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&) -> QueryDenotationView;

}  // namespace runir::kr::dl::semantics

namespace runir::kr::dl::semantics::incremental
{

template class EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>;
template class IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::GroundTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::prepare<ConceptTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::prepare<RoleTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::prepare<BooleanTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::prepare<NumericalTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>);

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::get_index<ConceptTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>) const;
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::get_index<RoleTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>) const;
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::get_index<BooleanTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>) const;
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::get_index<NumericalTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>) const;
template BorrowedDenotationView<ConceptTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::result<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template BorrowedDenotationView<RoleTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::result<RoleTag>(EvaluationIndex<RoleTag>) const;
template BorrowedDenotationView<BooleanTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::result<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template BorrowedDenotationView<NumericalTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::result<NumericalTag>(EvaluationIndex<NumericalTag>) const;
template const DenotationDelta<ConceptTag>& EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::change<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template const DenotationDelta<RoleTag>& EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::change<RoleTag>(EvaluationIndex<RoleTag>) const;
template size_t EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::cardinality<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template size_t EvaluationGraph<runir::kr::ExtFamilyTag, tyr::GroundTag>::cardinality<RoleTag>(EvaluationIndex<RoleTag>) const;
template bool IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::GroundTag>::changed<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template bool IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::GroundTag>::changed<NumericalTag>(EvaluationIndex<NumericalTag>) const;

template void IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::GroundTag>::demand<StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag>>(ygg::uint_t, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag>&, const Delta<runir::kr::ExtFamilyTag>*, ygg::database::Workspace*);
template void IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::GroundTag>::demand<StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>, BorrowedRegisterValuesView>>(ygg::uint_t, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>, BorrowedRegisterValuesView>&, const Delta<runir::kr::ExtFamilyTag>*, ygg::database::Workspace*);
template void IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::GroundTag>::demand<StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, BorrowedRegisterValuesView>>(ygg::uint_t, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::GroundTag, tyr::planning::StateView<tyr::GroundTag>, BorrowedRegisterValuesView>&, const Delta<runir::kr::ExtFamilyTag>*, ygg::database::Workspace*);

template class EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>;
template class IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::LiftedTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::prepare<ConceptTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::prepare<RoleTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::prepare<BooleanTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::prepare<NumericalTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>);

template EvaluationIndex<ConceptTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::get_index<ConceptTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, ConceptTag>) const;
template EvaluationIndex<RoleTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::get_index<RoleTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, RoleTag>) const;
template EvaluationIndex<BooleanTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::get_index<BooleanTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, BooleanTag>) const;
template EvaluationIndex<NumericalTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::get_index<NumericalTag>(FamilyConstructorView<runir::kr::ExtFamilyTag, NumericalTag>) const;
template BorrowedDenotationView<ConceptTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::result<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template BorrowedDenotationView<RoleTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::result<RoleTag>(EvaluationIndex<RoleTag>) const;
template BorrowedDenotationView<BooleanTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::result<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template BorrowedDenotationView<NumericalTag> EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::result<NumericalTag>(EvaluationIndex<NumericalTag>) const;
template const DenotationDelta<ConceptTag>& EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::change<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template const DenotationDelta<RoleTag>& EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::change<RoleTag>(EvaluationIndex<RoleTag>) const;
template size_t EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::cardinality<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template size_t EvaluationGraph<runir::kr::ExtFamilyTag, tyr::LiftedTag>::cardinality<RoleTag>(EvaluationIndex<RoleTag>) const;
template bool IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::LiftedTag>::changed<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template bool IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::LiftedTag>::changed<NumericalTag>(EvaluationIndex<NumericalTag>) const;

template void IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::LiftedTag>::demand<StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag>>(ygg::uint_t, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag>&, const Delta<runir::kr::ExtFamilyTag>*, ygg::database::Workspace*);
template void IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::LiftedTag>::demand<StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>, BorrowedRegisterValuesView>>(ygg::uint_t, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&, const Delta<runir::kr::ExtFamilyTag>*, ygg::database::Workspace*);
template void IncrementalEvaluation<runir::kr::ExtFamilyTag, tyr::LiftedTag>::demand<StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>, BorrowedRegisterValuesView>>(ygg::uint_t, StateEvaluationContext<runir::kr::ExtFamilyTag, tyr::LiftedTag, tyr::planning::StateView<tyr::LiftedTag>, BorrowedRegisterValuesView>&, const Delta<runir::kr::ExtFamilyTag>*, ygg::database::Workspace*);

}  // namespace runir::kr::dl::semantics::incremental
