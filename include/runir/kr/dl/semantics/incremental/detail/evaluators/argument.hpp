#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_ARGUMENT_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_ARGUMENT_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

namespace runir::kr::dl::semantics::incremental::detail
{

template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category>
struct ArgumentEvaluator
{
    ArgumentIdentifier<Category> identifier;
    DenotationState<Category> value;

    explicit ArgumentEvaluator(ArgumentIdentifier<Category> identifier) : identifier(identifier) {}
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>&, Context& context)
    {
        value.assign(context.arguments().at(identifier));
    }
    void update(EvaluationGraph<Family, Kind>&, const Delta<Family>&, ygg::database::Workspace<ObjectValues>&) {}
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
