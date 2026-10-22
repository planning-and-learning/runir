#include "runir/kr/dl/semantics/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/detail/repository_instantiations.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluation_impl.hpp"
#include "runir/kr/dl/semantics/state_evaluation.hpp"

namespace runir::kr::dl::semantics
{

template auto evaluate<tyr::GroundTag>(ConstructorView<ConceptTag>, StateEvaluationContext<tyr::GroundTag>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(ConstructorView<RoleTag>, StateEvaluationContext<tyr::GroundTag>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(ConstructorView<BooleanTag>, StateEvaluationContext<tyr::GroundTag>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(ConstructorView<NumericalTag>, StateEvaluationContext<tyr::GroundTag>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(QueryView<>, StateEvaluationContext<tyr::GroundTag>&) -> QueryDenotationView;

template auto evaluate<tyr::LiftedTag>(ConstructorView<ConceptTag>, StateEvaluationContext<tyr::LiftedTag>&) -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(ConstructorView<RoleTag>, StateEvaluationContext<tyr::LiftedTag>&) -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(ConstructorView<BooleanTag>, StateEvaluationContext<tyr::LiftedTag>&) -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(ConstructorView<NumericalTag>, StateEvaluationContext<tyr::LiftedTag>&) -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(QueryView<>, StateEvaluationContext<tyr::LiftedTag>&) -> QueryDenotationView;

template auto evaluate<tyr::GroundTag>(ConstructorView<ConceptTag>, StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&)
    -> DenotationView<ConceptTag>;
template auto evaluate<tyr::GroundTag>(ConstructorView<RoleTag>, StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&)
    -> DenotationView<RoleTag>;
template auto evaluate<tyr::GroundTag>(ConstructorView<BooleanTag>, StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&)
    -> DenotationView<BooleanTag>;
template auto evaluate<tyr::GroundTag>(ConstructorView<NumericalTag>, StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&)
    -> DenotationView<NumericalTag>;
template auto evaluate<tyr::GroundTag>(QueryView<>, StateEvaluationContext<tyr::GroundTag, tyr::planning::BuilderStateView<tyr::GroundTag>>&)
    -> QueryDenotationView;

template auto evaluate<tyr::LiftedTag>(ConstructorView<ConceptTag>, StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&)
    -> DenotationView<ConceptTag>;
template auto evaluate<tyr::LiftedTag>(ConstructorView<RoleTag>, StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&)
    -> DenotationView<RoleTag>;
template auto evaluate<tyr::LiftedTag>(ConstructorView<BooleanTag>, StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&)
    -> DenotationView<BooleanTag>;
template auto evaluate<tyr::LiftedTag>(ConstructorView<NumericalTag>, StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&)
    -> DenotationView<NumericalTag>;
template auto evaluate<tyr::LiftedTag>(QueryView<>, StateEvaluationContext<tyr::LiftedTag, tyr::planning::BuilderStateView<tyr::LiftedTag>>&)
    -> QueryDenotationView;

/// Queries over builder states with interned register values, which the successor expander tests evaluate.

}  // namespace runir::kr::dl::semantics

namespace runir::kr::dl::semantics::incremental
{

template class EvaluationGraph<tyr::GroundTag>;
template class IncrementalEvaluation<tyr::GroundTag>;

template EvaluationIndex<ConceptTag> EvaluationGraph<tyr::GroundTag>::prepare<ConceptTag>(ConstructorView<ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<tyr::GroundTag>::prepare<RoleTag>(ConstructorView<RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<tyr::GroundTag>::prepare<BooleanTag>(ConstructorView<BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<tyr::GroundTag>::prepare<NumericalTag>(ConstructorView<NumericalTag>);

template EvaluationIndex<ConceptTag> EvaluationGraph<tyr::GroundTag>::get_index<ConceptTag>(ConstructorView<ConceptTag>) const;
template EvaluationIndex<RoleTag> EvaluationGraph<tyr::GroundTag>::get_index<RoleTag>(ConstructorView<RoleTag>) const;
template EvaluationIndex<BooleanTag> EvaluationGraph<tyr::GroundTag>::get_index<BooleanTag>(ConstructorView<BooleanTag>) const;
template EvaluationIndex<NumericalTag> EvaluationGraph<tyr::GroundTag>::get_index<NumericalTag>(ConstructorView<NumericalTag>) const;
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

template EvaluationIndex<ConceptTag> EvaluationGraph<tyr::LiftedTag>::prepare<ConceptTag>(ConstructorView<ConceptTag>);
template EvaluationIndex<RoleTag> EvaluationGraph<tyr::LiftedTag>::prepare<RoleTag>(ConstructorView<RoleTag>);
template EvaluationIndex<BooleanTag> EvaluationGraph<tyr::LiftedTag>::prepare<BooleanTag>(ConstructorView<BooleanTag>);
template EvaluationIndex<NumericalTag> EvaluationGraph<tyr::LiftedTag>::prepare<NumericalTag>(ConstructorView<NumericalTag>);

template EvaluationIndex<ConceptTag> EvaluationGraph<tyr::LiftedTag>::get_index<ConceptTag>(ConstructorView<ConceptTag>) const;
template EvaluationIndex<RoleTag> EvaluationGraph<tyr::LiftedTag>::get_index<RoleTag>(ConstructorView<RoleTag>) const;
template EvaluationIndex<BooleanTag> EvaluationGraph<tyr::LiftedTag>::get_index<BooleanTag>(ConstructorView<BooleanTag>) const;
template EvaluationIndex<NumericalTag> EvaluationGraph<tyr::LiftedTag>::get_index<NumericalTag>(ConstructorView<NumericalTag>) const;
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
