#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_NUMBER_RESTRICTION_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_NUMBER_RESTRICTION_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/incremental/detail/set_operations.hpp"

namespace runir::kr::dl::semantics::incremental::detail
{

template<FamilyTag Family, tyr::TaskKind Kind, typename Tag>
struct NumberRestrictionEvaluator
{
    EvaluationIndex<RoleTag> role;
    ygg::uint_t threshold;
    DenotationState<ConceptTag> value;

    NumberRestrictionEvaluator(EvaluationIndex<RoleTag> role, ygg::uint_t threshold) : role(role), threshold(threshold) {}
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        initialize_set(Tag {}, value, graph.result(role), graph.change(role), threshold, graph.set_workspace());
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ObjectValues>&)
    {
        update_set(Tag {}, value, graph.result(role), graph.change(role), threshold, graph.set_workspace());
    }
};

template<FamilyTag Family, tyr::TaskKind Kind, typename Tag>
struct QualifiedNumberRestrictionEvaluator
{
    EvaluationIndex<RoleTag> role;
    EvaluationIndex<ConceptTag> qualifying_concept;
    ygg::uint_t threshold;
    DenotationState<ConceptTag> value;

    QualifiedNumberRestrictionEvaluator(EvaluationIndex<RoleTag> role, EvaluationIndex<ConceptTag> qualifying_concept, ygg::uint_t threshold) :
        role(role),
        qualifying_concept(qualifying_concept),
        threshold(threshold)
    {
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        initialize_set(Tag {},
                       value,
                       graph.result(role),
                       graph.change(role),
                       graph.result(qualifying_concept),
                       graph.change(qualifying_concept),
                       threshold,
                       graph.set_workspace());
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ObjectValues>&)
    {
        update_set(Tag {},
                   value,
                   graph.result(role),
                   graph.change(role),
                   graph.result(qualifying_concept),
                   graph.change(qualifying_concept),
                   threshold,
                   graph.set_workspace());
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
