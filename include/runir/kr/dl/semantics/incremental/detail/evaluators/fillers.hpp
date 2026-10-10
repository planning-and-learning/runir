#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_FILLERS_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_FILLERS_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/incremental/detail/set_operations.hpp"

namespace runir::kr::dl::semantics::incremental::detail
{

template<FamilyTag Family, tyr::TaskKind Kind>
struct FillersEvaluator
{
    EvaluationIndex<RoleTag> role;
    FamilyConceptView<Family, RoleFillersTag> expression;
    DenotationState<ConceptTag> value;

    FillersEvaluator(EvaluationIndex<RoleTag> role, FamilyConceptView<Family, RoleFillersTag> expression) : role(role), expression(expression) {}
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        initialize_set(RoleFillersTag {}, value, graph.result(role), graph.change(role), expression.get_objects(), graph.set_workspace());
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<QueryValues>&)
    {
        update_set(RoleFillersTag {}, value, graph.result(role), graph.change(role), expression.get_objects(), graph.set_workspace());
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
