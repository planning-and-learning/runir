#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_SET_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_SET_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/incremental/detail/set_operations.hpp"

namespace runir::kr::dl::semantics::incremental::detail
{

template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category, typename Tag, ConceptOrRoleTag Input>
struct UnarySetEvaluator
{
    EvaluationIndex<Input> child;
    DenotationState<Category> value;

    explicit UnarySetEvaluator(EvaluationIndex<Input> child) : child(child) {}
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        initialize_set(Tag {}, value, graph.result(child), graph.change(child), graph.set_workspace());
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ObjectValues>&)
    {
        update_set(Tag {}, value, graph.result(child), graph.change(child), graph.set_workspace());
    }
};

template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category, typename Tag, ConceptOrRoleTag Left, ConceptOrRoleTag Right>
struct BinarySetEvaluator
{
    EvaluationIndex<Left> lhs;
    EvaluationIndex<Right> rhs;
    DenotationState<Category> value;

    BinarySetEvaluator(EvaluationIndex<Left> lhs, EvaluationIndex<Right> rhs) : lhs(lhs), rhs(rhs) {}
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        initialize_set(Tag {}, value, graph.result(lhs), graph.change(lhs), graph.result(rhs), graph.change(rhs), graph.set_workspace());
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ObjectValues>&)
    {
        update_set(Tag {}, value, graph.result(lhs), graph.change(lhs), graph.result(rhs), graph.change(rhs), graph.set_workspace());
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
