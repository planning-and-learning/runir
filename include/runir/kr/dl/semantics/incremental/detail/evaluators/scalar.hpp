#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_SCALAR_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_SCALAR_HPP_

#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <concepts>

namespace runir::kr::dl::semantics::incremental::detail
{

template<tyr::TaskKind Kind, CategoryTag Category, typename Tag, BooleanOrNumericalTag Input>
struct ScalarBinaryEvaluator
{
    EvaluationIndex<Input> left;
    EvaluationIndex<Input> right;
    DenotationState<Category> value;

    ScalarBinaryEvaluator(EvaluationIndex<Input> left_, EvaluationIndex<Input> right_) : left(left_), right(right_) {}
    void refresh(EvaluationGraph<Kind>& graph)
    {
        if constexpr (NumericalBinaryTag<Tag>)
            value.set(semantics::detail::apply_numerical_binary<Tag>(graph.result(left).value(), graph.result(right).value()));
        else if constexpr (LogicalBinaryTag<Tag>)
            value.set(semantics::detail::apply_logical_binary<Tag>(graph.result(left).value(), graph.result(right).value()));
        else
            value.set(semantics::detail::apply_comparison<Tag>(graph.result(left).value(), graph.result(right).value()));
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    { refresh(graph); }
    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace&) { refresh(graph); }
};

template<tyr::TaskKind Kind>
struct LogicalNotEvaluator
{
    EvaluationIndex<BooleanTag> child;
    DenotationState<BooleanTag> value;

    explicit LogicalNotEvaluator(EvaluationIndex<BooleanTag> child_) : child(child_) {}
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    { value.set(!graph.result(child).value()); }
    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace&) { value.set(!graph.result(child).value()); }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
