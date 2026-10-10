#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DISTANCE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DISTANCE_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/incremental/detail/distance.hpp"

namespace runir::kr::dl::semantics::incremental::detail
{

template<FamilyTag Family, tyr::TaskKind Kind>
struct DistanceFeatureEvaluator
{
    EvaluationIndex<ConceptTag> sources;
    EvaluationIndex<RoleTag> edges;
    EvaluationIndex<ConceptTag> targets;
    DistanceEvaluator operation;
    DenotationState<NumericalTag> value;

    DistanceFeatureEvaluator(EvaluationIndex<ConceptTag> sources, EvaluationIndex<RoleTag> edges, EvaluationIndex<ConceptTag> targets) :
        sources(sources),
        edges(edges),
        targets(targets)
    {
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        operation.initialize(graph.result(sources), graph.result(edges), graph.result(targets));
        value.set(operation.get_result());
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ObjectValues>&)
    {
        operation.update(graph.result(sources), graph.result(edges), graph.result(targets), graph.change(sources), graph.change(edges), graph.change(targets));
        value.set(operation.get_result());
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
