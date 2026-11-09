#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_PROJECTION_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_PROJECTION_HPP_

#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <concepts>
#include <span>
#include <tyr/datalog/lifted/database/semantics/incremental/projection.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

template<tyr::TaskKind Kind, CategoryTag Category>
struct ProjectionEvaluator
{
    QueryEvaluationIndex child;
    tyr::datalog::database::incremental::ProjectionEvaluator operation;
    DenotationState<Category> value;

    ProjectionEvaluator(const ygg::FreeRelationRepository& layer, QueryEvaluationIndex child_, const tyr::datalog::database::ProjectionPlan& plan) :
        child(child_),
        operation(layer, plan)
    {
    }
    static void set_row(urd::builder<Denotation<Category>>& builder, const auto& row, bool present)
    {
        using ObjectIndex = urd::index<tyr::formalism::Object>;
        if constexpr (std::same_as<Category, ConceptTag>)
            builder.set(ObjectIndex(ygg::decode_value<ygg::uint_t>(row.value(0))), present);
        else
            builder.set(ObjectIndex(ygg::decode_value<ygg::uint_t>(row.value(0))), ObjectIndex(ygg::decode_value<ygg::uint_t>(row.value(1))), present);
    }
    static void set_row(DenotationState<Category>& state, const auto& row, bool present, const tyr::formalism::planning::Repository& repository)
    {
        using ObjectIndex = urd::index<tyr::formalism::Object>;
        if constexpr (std::same_as<Category, ConceptTag>)
            state.set(ObjectIndex(ygg::decode_value<ygg::uint_t>(row.value(0))), present, repository);
        else
            state.set(ObjectIndex(ygg::decode_value<ygg::uint_t>(row.value(0))), ObjectIndex(ygg::decode_value<ygg::uint_t>(row.value(1))), present, repository);
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context& context)
    {
        auto& builder = value.initialize(semantics::detail::num_objects<Kind>(context));
        operation.initialize(graph.result(child), context.get_workspace().get_database_workspace());
        const auto rows = operation.get_result();
        for (size_t i = 0; i < rows.size(); ++i)
            set_row(builder, rows.row(i), true);
    }
    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace& workspace)
    {
        operation.update(graph.change(child).change(), workspace);
        const auto [added, removed] = operation.get_delta().change();
        for (size_t i = 0; i < removed.size(); ++i)
            set_row(value, removed.row(i), false, graph.repository());
        for (size_t i = 0; i < added.size(); ++i)
            set_row(value, added.row(i), true, graph.repository());
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
