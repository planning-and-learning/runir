#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_PROJECTION_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_PROJECTION_HPP_

#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <concepts>
#include <runir/kr/database/semantics/incremental/projection.hpp>
#include <span>

namespace runir::kr::dl::semantics::incremental::detail
{

template<tyr::TaskKind Kind, CategoryTag Category>
struct ProjectionEvaluator
{
    QueryEvaluationIndex child;
    runir::kr::database::incremental::ProjectionEvaluator operation;
    DenotationState<Category> value;

    ProjectionEvaluator(ygg::FreeScratchRelations& scratch, QueryEvaluationIndex child_, const urd::data<runir::kr::database::ProjectionPlan>& plan) :
        child(child_),
        operation(scratch, plan)
    {
    }

    static void set_row(urd::builder<Denotation<Category>, runir::kr::dl::semantics::DenotationRepository>& builder, const auto& row, bool present)
    {
        using ObjectIndex = urd::index<tyr::formalism::Object>;
        if constexpr (std::same_as<Category, ConceptTag>)
            builder.set(ObjectIndex(ygg::decode_value<ygg::uint_t>(ygg::value_of(row, 0))), present);
        else
            builder.set(ObjectIndex(ygg::decode_value<ygg::uint_t>(ygg::value_of(row, 0))),
                        ObjectIndex(ygg::decode_value<ygg::uint_t>(ygg::value_of(row, 1))),
                        present);
    }

    static void set_row(DenotationState<Category>& state, const auto& row, bool present, const tyr::formalism::Repository& repository)
    {
        using ObjectIndex = urd::index<tyr::formalism::Object>;
        if constexpr (std::same_as<Category, ConceptTag>)
            state.set(ObjectIndex(ygg::decode_value<ygg::uint_t>(ygg::value_of(row, 0))), present, repository);
        else
            state.set(ObjectIndex(ygg::decode_value<ygg::uint_t>(ygg::value_of(row, 0))),
                      ObjectIndex(ygg::decode_value<ygg::uint_t>(ygg::value_of(row, 1))),
                      present,
                      repository);
    }

    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context& context)
    {
        auto& builder = value.initialize(semantics::detail::num_objects<Kind>(context), graph.records(), graph.denotations());
        operation.initialize(graph.result(child), context.get_workspace().get_database_workspace());
        const auto rows = operation.get_result();
        for (size_t i = 0; i < rows.rows().size(); ++i)
            set_row(builder, rows.row(i), true);
    }

    void update(EvaluationGraph<Kind>& graph, const Delta&, runir::kr::database::Workspace& workspace)
    {
        operation.update(graph.change(child).change(), workspace);
        const auto [added, removed] = operation.get_delta().change();
        for (size_t i = 0; i < removed.rows().size(); ++i)
            set_row(value, removed.row(i), false, graph.repository());
        for (size_t i = 0; i < added.rows().size(); ++i)
            set_row(value, added.row(i), true, graph.repository());
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
