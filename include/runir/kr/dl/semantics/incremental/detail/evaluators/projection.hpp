#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_PROJECTION_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_PROJECTION_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

#include <concepts>
#include <span>
#include <yggdrasil/database/incremental/projection.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category>
struct ProjectionEvaluator
{
    QueryEvaluationIndex child;
    ygg::database::incremental::ProjectionEvaluator<ygg::Index<tyr::formalism::Object>> operation;
    DenotationState<Category> value;

    ProjectionEvaluator(QueryEvaluationIndex child, const ygg::database::ProjectionPlan& plan) : child(child), operation(plan) {}
    void set_row(EvaluationGraph<Family, Kind>& graph, std::span<const ygg::Index<tyr::formalism::Object>> row, bool present)
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            value.set(row[0], present, graph.repository());
        else
            value.set(row[0], row[1], present, graph.repository());
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
    {
        auto& builder = value.initialize(semantics::detail::num_objects<Kind, Family>(context));
        operation.initialize(graph.result(child), context.get_workspace().get_database_workspace());
        const auto& rows = operation.get_result();
        for (size_t i = 0; i < rows.size(); ++i)
        {
            const auto row = rows.row(i);
            if constexpr (std::same_as<Category, ConceptTag>)
                builder.set(row[0], true);
            else
                builder.set(row[0], row[1], true);
        }
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ygg::Index<tyr::formalism::Object>>& workspace)
    {
        const auto& input = graph.change(child);
        operation.update(input.added, input.removed, workspace);
        const auto& change = operation.get_delta();
        for (size_t i = 0; i < change.removed.size(); ++i)
            set_row(graph, change.removed.row(i), false);
        for (size_t i = 0; i < change.added.size(); ++i)
            set_row(graph, change.added.row(i), true);
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
