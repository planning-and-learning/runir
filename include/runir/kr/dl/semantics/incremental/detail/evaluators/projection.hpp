#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_PROJECTION_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_PROJECTION_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

#include <concepts>
#include <span>
#include <yggdrasil/database/semantics/incremental/projection.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

template<FamilyTag Family, tyr::TaskKind Kind, CategoryTag Category>
struct ProjectionEvaluator
{
    QueryEvaluationIndex child;
    ygg::database::incremental::ProjectionEvaluator<ObjectValues> operation;
    DenotationState<Category> value;

    ProjectionEvaluator(QueryEvaluationIndex child, const ygg::database::ProjectionPlan<ObjectValues>& plan) : child(child), operation(plan) {}
    template<typename Builder>
    static void set_row(Builder& builder, ygg::database::Row<ObjectValues> row, bool present, auto&&... context)
    {
        using ObjectIndex = ygg::Index<tyr::formalism::Object>;
        if constexpr (std::same_as<Category, ConceptTag>)
            builder.set(row.get<ObjectIndex>(size_t { 0 }), present, context...);
        else
            builder.set(row.get<ObjectIndex>(size_t { 0 }), row.get<ObjectIndex>(size_t { 1 }), present, context...);
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
    {
        auto& builder = value.initialize(semantics::detail::num_objects<Kind, Family>(context));
        operation.initialize(graph.result(child), context.get_workspace().get_database_workspace());
        const auto& rows = operation.get_result();
        for (size_t i = 0; i < rows.size(); ++i)
            set_row(builder, rows[i], true);
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<ObjectValues>& workspace)
    {
        operation.update(graph.change(child).change(), workspace);
        const auto& change = operation.get_delta();
        for (size_t i = 0; i < change.removed.size(); ++i)
            set_row(value, change.removed[i], false, graph.repository());
        for (size_t i = 0; i < change.added.size(); ++i)
            set_row(value, change.added[i], true, graph.repository());
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
