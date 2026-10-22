#include "runir/kr/dl/semantics/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/detail/repository_instantiations.hpp"
#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"

namespace runir::kr::dl::semantics
{

template auto evaluate<tyr::GroundTag>(FamilyConstructorView<ConceptTag>, StateEvaluationContext<tyr::GroundTag>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<RoleTag>, StateEvaluationContext<tyr::GroundTag>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<BooleanTag>, StateEvaluationContext<tyr::GroundTag>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<NumericalTag>, StateEvaluationContext<tyr::GroundTag>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(FamilyQueryView<>, StateEvaluationContext<tyr::GroundTag>&) -> QueryDenotationView;

template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<ConceptTag>, StateEvaluationContext<tyr::LiftedTag>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<RoleTag>, StateEvaluationContext<tyr::LiftedTag>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<BooleanTag>, StateEvaluationContext<tyr::LiftedTag>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<NumericalTag>, StateEvaluationContext<tyr::LiftedTag>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(FamilyQueryView<>, StateEvaluationContext<tyr::LiftedTag>&) -> QueryDenotationView;

template auto evaluate<tyr::GroundTag>(FamilyConstructorView<ConceptTag>,
                                       StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<RoleTag>, StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&)
    -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<BooleanTag>,
                                       StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(FamilyConstructorView<NumericalTag>,
                                       StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&)
    -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(FamilyQueryView<>, StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&)
    -> QueryDenotationView;

template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<ConceptTag>,
                                       StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<RoleTag>, StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&)
    -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<BooleanTag>,
                                       StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(FamilyConstructorView<NumericalTag>,
                                       StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&)
    -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(FamilyQueryView<>, StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&)
    -> QueryDenotationView;

/// Queries over builder states with interned register values, which the successor expander tests evaluate.

}  // namespace runir::kr::dl::semantics

namespace runir::kr::dl::semantics::incremental
{

template class EvaluationGraph<tyr::GroundTag>;
template class IncrementalEvaluation<tyr::GroundTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<tyr::GroundTag>::prepare<ConceptTag>(FamilyConstructorView<ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<tyr::GroundTag>::prepare<RoleTag>(FamilyConstructorView<RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<tyr::GroundTag>::prepare<BooleanTag>(FamilyConstructorView<BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<tyr::GroundTag>::prepare<NumericalTag>(FamilyConstructorView<NumericalTag>);

template EvaluationIndex<ConceptTag> EvaluationGraph<tyr::GroundTag>::get_index<ConceptTag>(FamilyConstructorView<ConceptTag>) const;
template EvaluationIndex<RoleTag> EvaluationGraph<tyr::GroundTag>::get_index<RoleTag>(FamilyConstructorView<RoleTag>) const;
template EvaluationIndex<BooleanTag> EvaluationGraph<tyr::GroundTag>::get_index<BooleanTag>(FamilyConstructorView<BooleanTag>) const;
template EvaluationIndex<NumericalTag> EvaluationGraph<tyr::GroundTag>::get_index<NumericalTag>(FamilyConstructorView<NumericalTag>) const;
template BorrowedDenotationView<ConceptTag> EvaluationGraph<tyr::GroundTag>::result<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template BorrowedDenotationView<RoleTag> EvaluationGraph<tyr::GroundTag>::result<RoleTag>(EvaluationIndex<RoleTag>) const;
template BorrowedDenotationView<BooleanTag> EvaluationGraph<tyr::GroundTag>::result<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template BorrowedDenotationView<NumericalTag> EvaluationGraph<tyr::GroundTag>::result<NumericalTag>(EvaluationIndex<NumericalTag>) const;
template const DenotationDelta<ConceptTag>& EvaluationGraph<tyr::GroundTag>::change<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template const DenotationDelta<RoleTag>& EvaluationGraph<tyr::GroundTag>::change<RoleTag>(EvaluationIndex<RoleTag>) const;
template size_t EvaluationGraph<tyr::GroundTag>::cardinality<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template size_t EvaluationGraph<tyr::GroundTag>::cardinality<RoleTag>(EvaluationIndex<RoleTag>) const;
template bool IncrementalEvaluation<tyr::GroundTag>::changed<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template bool IncrementalEvaluation<tyr::GroundTag>::changed<NumericalTag>(EvaluationIndex<NumericalTag>) const;

template void IncrementalEvaluation<tyr::GroundTag>::demand<StateEvaluationContext<tyr::GroundTag>>(ygg::uint_t,
                                                                                                    StateEvaluationContext<tyr::GroundTag>&,
                                                                                                    const Delta*,
                                                                                                    ygg::database::Workspace*);
template void IncrementalEvaluation<tyr::GroundTag>::demand<StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>>(
    ygg::uint_t,
    StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&,
    const Delta*,
    ygg::database::Workspace*);

template class EvaluationGraph<tyr::LiftedTag>;
template class IncrementalEvaluation<tyr::LiftedTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<tyr::LiftedTag>::prepare<ConceptTag>(FamilyConstructorView<ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<tyr::LiftedTag>::prepare<RoleTag>(FamilyConstructorView<RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<tyr::LiftedTag>::prepare<BooleanTag>(FamilyConstructorView<BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<tyr::LiftedTag>::prepare<NumericalTag>(FamilyConstructorView<NumericalTag>);

template EvaluationIndex<ConceptTag> EvaluationGraph<tyr::LiftedTag>::get_index<ConceptTag>(FamilyConstructorView<ConceptTag>) const;
template EvaluationIndex<RoleTag> EvaluationGraph<tyr::LiftedTag>::get_index<RoleTag>(FamilyConstructorView<RoleTag>) const;
template EvaluationIndex<BooleanTag> EvaluationGraph<tyr::LiftedTag>::get_index<BooleanTag>(FamilyConstructorView<BooleanTag>) const;
template EvaluationIndex<NumericalTag> EvaluationGraph<tyr::LiftedTag>::get_index<NumericalTag>(FamilyConstructorView<NumericalTag>) const;
template BorrowedDenotationView<ConceptTag> EvaluationGraph<tyr::LiftedTag>::result<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template BorrowedDenotationView<RoleTag> EvaluationGraph<tyr::LiftedTag>::result<RoleTag>(EvaluationIndex<RoleTag>) const;
template BorrowedDenotationView<BooleanTag> EvaluationGraph<tyr::LiftedTag>::result<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template BorrowedDenotationView<NumericalTag> EvaluationGraph<tyr::LiftedTag>::result<NumericalTag>(EvaluationIndex<NumericalTag>) const;
template const DenotationDelta<ConceptTag>& EvaluationGraph<tyr::LiftedTag>::change<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template const DenotationDelta<RoleTag>& EvaluationGraph<tyr::LiftedTag>::change<RoleTag>(EvaluationIndex<RoleTag>) const;
template size_t EvaluationGraph<tyr::LiftedTag>::cardinality<ConceptTag>(EvaluationIndex<ConceptTag>) const;
template size_t EvaluationGraph<tyr::LiftedTag>::cardinality<RoleTag>(EvaluationIndex<RoleTag>) const;
template bool IncrementalEvaluation<tyr::LiftedTag>::changed<BooleanTag>(EvaluationIndex<BooleanTag>) const;
template bool IncrementalEvaluation<tyr::LiftedTag>::changed<NumericalTag>(EvaluationIndex<NumericalTag>) const;

template void IncrementalEvaluation<tyr::LiftedTag>::demand<StateEvaluationContext<tyr::LiftedTag>>(ygg::uint_t,
                                                                                                    StateEvaluationContext<tyr::LiftedTag>&,
                                                                                                    const Delta*,
                                                                                                    ygg::database::Workspace*);
template void IncrementalEvaluation<tyr::LiftedTag>::demand<StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>>(
    ygg::uint_t,
    StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&,
    const Delta*,
    ygg::database::Workspace*);

}  // namespace runir::kr::dl::semantics::incremental
