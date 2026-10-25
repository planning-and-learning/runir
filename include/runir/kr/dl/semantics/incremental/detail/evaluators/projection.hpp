#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_PROJECTION_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_PROJECTION_HPP_

#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/state_evaluation.hpp"

#include <concepts>
#include <span>
#include <yggdrasil/database/semantics/incremental/projection.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

template<tyr::TaskKind Kind, CategoryTag Category>
struct ProjectionEvaluator
{
    QueryEvaluationIndex child;
    ygg::database::incremental::ProjectionEvaluator operation;
    DenotationState<Category> value;

    ProjectionEvaluator(QueryEvaluationIndex child, const ygg::database::ProjectionPlan& plan) : child(child), operation(plan) {}
    static void set_row(ygg::Builder<Denotation<Category>>& builder, ygg::database::Row row, bool present)
    {
        using ObjectIndex = ygg::Index<tyr::formalism::Object>;
        if constexpr (std::same_as<Category, ConceptTag>)
            builder.set(ObjectIndex(row.template get<ygg::uint_t>(size_t { 0 })), present);
        else
            builder.set(ObjectIndex(row.template get<ygg::uint_t>(size_t { 0 })), ObjectIndex(row.template get<ygg::uint_t>(size_t { 1 })), present);
    }
    static void set_row(DenotationState<Category>& state, ygg::database::Row row, bool present, const tyr::formalism::planning::Repository& repository)
    {
        using ObjectIndex = ygg::Index<tyr::formalism::Object>;
        if constexpr (std::same_as<Category, ConceptTag>)
            state.set(ObjectIndex(row.template get<ygg::uint_t>(size_t { 0 })), present, repository);
        else
            state.set(ObjectIndex(row.template get<ygg::uint_t>(size_t { 0 })), ObjectIndex(row.template get<ygg::uint_t>(size_t { 1 })), present, repository);
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context& context)
    {
        auto& builder = value.initialize(semantics::detail::num_objects<Kind>(context));
        operation.initialize(graph.result(child), context.get_workspace().get_database_workspace());
        const auto& rows = operation.get_result();
        for (size_t i = 0; i < rows.size(); ++i)
            set_row(builder, rows[i], true);
    }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace& workspace)
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
