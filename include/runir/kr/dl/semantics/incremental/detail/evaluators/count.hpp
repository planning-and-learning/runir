#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_COUNT_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_COUNT_HPP_

#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/state_evaluation.hpp"

#include <cstddef>
#include <variant>

namespace runir::kr::dl::semantics::incremental::detail
{

template<tyr::TaskKind Kind>
struct CountEvaluator
{
    std::variant<EvaluationIndex<ConceptTag>, EvaluationIndex<RoleTag>, QueryEvaluationIndex> child;
    DenotationState<NumericalTag> value;

    explicit CountEvaluator(std::variant<EvaluationIndex<ConceptTag>, EvaluationIndex<RoleTag>, QueryEvaluationIndex> child_) : child(child_) {}
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        value.set(ygg::to_uint_t(std::visit([&](auto index) { return graph.cardinality(index); }, child)));
    }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
    {
        value.set(std::visit(
            [&](auto index)
            {
                const auto& delta = graph.change(index);
                // The numerical result retains the count; only net membership changes are needed.
                const auto remaining = static_cast<size_t>(value.get_value()) - delta.removed.size();
                return ygg::to_uint_t(remaining + delta.added.size());
            },
            child));
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
