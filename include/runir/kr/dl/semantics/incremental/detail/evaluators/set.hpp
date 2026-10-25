#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_SET_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_SET_HPP_

#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/incremental/detail/set_operations.hpp"
#include "runir/kr/dl/semantics/state_evaluation.hpp"

namespace runir::kr::dl::semantics::incremental::detail
{

template<tyr::TaskKind Kind, CategoryTag Category, typename Tag, ConceptOrRoleTag Input>
struct UnarySetEvaluator
{
    EvaluationIndex<Input> child;
    DenotationState<Category> value;

    explicit UnarySetEvaluator(EvaluationIndex<Input> child_) : child(child_) {}
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    { initialize_set(Tag {}, value, graph.result(child), graph.change(child), graph.set_workspace()); }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
    { update_set(Tag {}, value, graph.result(child), graph.change(child), graph.set_workspace()); }
};

template<tyr::TaskKind Kind, CategoryTag Category, typename Tag, ConceptOrRoleTag Left, ConceptOrRoleTag Right>
struct BinarySetEvaluator
{
    EvaluationIndex<Left> lhs;
    EvaluationIndex<Right> rhs;
    DenotationState<Category> value;

    BinarySetEvaluator(EvaluationIndex<Left> lhs_, EvaluationIndex<Right> rhs_) : lhs(lhs_), rhs(rhs_) {}
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    { initialize_set(Tag {}, value, graph.result(lhs), graph.change(lhs), graph.result(rhs), graph.change(rhs), graph.set_workspace()); }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
    { update_set(Tag {}, value, graph.result(lhs), graph.change(lhs), graph.result(rhs), graph.change(rhs), graph.set_workspace()); }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
