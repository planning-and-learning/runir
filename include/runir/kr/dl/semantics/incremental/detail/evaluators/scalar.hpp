#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_SCALAR_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_SCALAR_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

#include <concepts>

namespace runir::kr::dl::semantics::incremental::detail
{

template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category, typename Tag, BooleanOrNumericalTag Input>
struct ScalarBinaryEvaluator
{
    EvaluationIndex<Input> lhs;
    EvaluationIndex<Input> rhs;
    DenotationState<Category> value;

    ScalarBinaryEvaluator(EvaluationIndex<Input> lhs, EvaluationIndex<Input> rhs) : lhs(lhs), rhs(rhs) {}
    void refresh(EvaluationGraph<Family, Kind>& graph)
    {
        if constexpr (NumericalBinaryTag<Tag>)
            value.set(semantics::detail::apply_numerical_binary<Tag>(graph.result(lhs).get(), graph.result(rhs).get()));
        else if constexpr (LogicalBinaryTag<Tag>)
            value.set(semantics::detail::apply_logical_binary<Tag>(graph.result(lhs).get(), graph.result(rhs).get()));
        else
            value.set(semantics::detail::apply_comparison<Tag>(graph.result(lhs).get(), graph.result(rhs).get()));
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        refresh(graph);
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&) { refresh(graph); }
};

template<FamilyTag Family, tyr::TaskKind Kind>
struct LogicalNotEvaluator
{
    EvaluationIndex<BooleanTag> child;
    DenotationState<BooleanTag> value;

    explicit LogicalNotEvaluator(EvaluationIndex<BooleanTag> child) : child(child) {}
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        value.set(!graph.result(child).get());
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
    {
        value.set(!graph.result(child).get());
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
