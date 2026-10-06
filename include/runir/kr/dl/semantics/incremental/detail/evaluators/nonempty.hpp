#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_NONEMPTY_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_NONEMPTY_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

#include <variant>

namespace runir::kr::dl::semantics::incremental::detail
{

template<FamilyTag Family, tyr::TaskKind Kind>
struct NonemptyEvaluator
{
    std::variant<EvaluationIndex<ConceptTag>, EvaluationIndex<RoleTag>, QueryEvaluationIndex> child;
    DenotationState<BooleanTag> value;

    explicit NonemptyEvaluator(std::variant<EvaluationIndex<ConceptTag>, EvaluationIndex<RoleTag>, QueryEvaluationIndex> child) : child(child) {}
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        value.set(std::visit([&](auto index) { return graph.nonempty(index); }, child));
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>&)
    {
        value.set(std::visit(
            [&](auto index)
            {
                const auto& delta = graph.change(index);
                if (!delta.added.empty())
                    return true;
                if (delta.removed.empty())
                    return value.get_value();
                return graph.nonempty(index);
            },
            child));
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
