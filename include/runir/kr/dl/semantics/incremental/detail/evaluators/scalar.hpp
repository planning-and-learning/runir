#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_SCALAR_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_SCALAR_HPP_

#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/state_evaluation.hpp"

#include <concepts>

namespace runir::kr::dl::semantics::incremental::detail
{

template<tyr::TaskKind Kind, CategoryTag Category, typename Tag, BooleanOrNumericalTag Input>
struct ScalarBinaryEvaluator
{
    EvaluationIndex<Input> lhs;
    EvaluationIndex<Input> rhs;
    DenotationState<Category> value;

    ScalarBinaryEvaluator(EvaluationIndex<Input> lhs_, EvaluationIndex<Input> rhs_) : lhs(lhs_), rhs(rhs_) {}
    void refresh(EvaluationGraph<Kind>& graph)
    {
        if constexpr (NumericalBinaryTag<Tag>)
            value.set(semantics::detail::apply_numerical_binary<Tag>(graph.result(lhs).get(), graph.result(rhs).get()));
        else if constexpr (LogicalBinaryTag<Tag>)
            value.set(semantics::detail::apply_logical_binary<Tag>(graph.result(lhs).get(), graph.result(rhs).get()));
        else
            value.set(semantics::detail::apply_comparison<Tag>(graph.result(lhs).get(), graph.result(rhs).get()));
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    { refresh(graph); }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&) { refresh(graph); }
};

template<tyr::TaskKind Kind>
struct LogicalNotEvaluator
{
    EvaluationIndex<BooleanTag> child;
    DenotationState<BooleanTag> value;

    explicit LogicalNotEvaluator(EvaluationIndex<BooleanTag> child_) : child(child_) {}
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    { value.set(!graph.result(child).get()); }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&) { value.set(!graph.result(child).get()); }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
