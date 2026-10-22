#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_STATIC_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_STATIC_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

namespace runir::kr::dl::semantics::incremental::detail
{

template<tyr::TaskKind Kind, CategoryTag Category>
struct StaticEvaluator
{
    FamilyConstructorView<Category> expression;
    DenotationState<Category> value;
    bool initialized = false;

    explicit StaticEvaluator(FamilyConstructorView<Category> expression) : expression(expression) {}
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>&, Context& context)
    {
        if (!initialized)
        {
            value.assign(semantics::evaluate<Kind>(expression, context));
            initialized = true;
        }
    }
    void update(EvaluationGraph<Kind>&, const Delta&, ygg::database::Workspace&) {}
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
