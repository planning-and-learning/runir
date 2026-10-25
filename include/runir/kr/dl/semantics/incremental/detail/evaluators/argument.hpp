#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_ARGUMENT_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_ARGUMENT_HPP_

#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/state_evaluation.hpp"

namespace runir::kr::dl::semantics::incremental::detail
{

template<tyr::TaskKind Kind, CategoryTag Category>
struct ArgumentEvaluator
{
    ArgumentIdentifier<Category> identifier;
    DenotationState<Category> value;

    explicit ArgumentEvaluator(ArgumentIdentifier<Category> identifier) : identifier(identifier) {}
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>&, Context& context)
    { value.assign(argument_value(context, identifier)); }
    void update(EvaluationGraph<Kind>&, const Delta&, ygg::database::Workspace&) {}
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
