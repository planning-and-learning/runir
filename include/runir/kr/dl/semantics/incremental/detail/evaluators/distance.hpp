#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DISTANCE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DISTANCE_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

#include <span>
#include <vector>
#include <yggdrasil/database/semantics/incremental/distance.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

/// Wiring only: every argument is a relation in the graph (a query, or a concept or role lifted by
/// the graph), and yggdrasil maintains the minimum distance over them.
template<FamilyTag Family, tyr::TaskKind Kind>
struct DistanceFeatureEvaluator
{
    QueryEvaluationIndex sources;
    QueryEvaluationIndex edges;
    QueryEvaluationIndex targets;
    ygg::database::incremental::MinDistanceEvaluator<QueryValues> operation;
    DenotationState<NumericalTag> value;

    template<typename C>
    DistanceFeatureEvaluator(ygg::View<ygg::Index<FamilyNumerical<Family, DistanceTag>>, C> expression,
                             EvaluationGraph<Family, Kind>& graph,
                             std::vector<ygg::uint_t>& dependencies) :
        sources(prepare(expression.get_lhs(), expression.get_data().plan.source_columns().span(), graph, dependencies)),
        edges(prepare(expression.get_mid(), expression.get_data().plan.edge_columns().span(), graph, dependencies)),
        targets(prepare(expression.get_rhs(), expression.get_data().plan.target_columns().span(), graph, dependencies)),
        operation(expression.get_data().plan)
    {
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context&)
    {
        operation.initialize(graph.result(sources), graph.result(edges), graph.result(targets));
        value.set(operation.get_result());
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<QueryValues>&)
    {
        operation.update(graph.change(sources).change(), graph.change(edges).change(), graph.change(targets).change());
        value.set(operation.get_result());
    }

private:
    template<typename Operand>
    static QueryEvaluationIndex prepare(Operand operand,
                                        std::span<const ygg::database::ColumnLayout> columns,
                                        EvaluationGraph<Family, Kind>& graph,
                                        std::vector<ygg::uint_t>& dependencies)
    {
        return ygg::visit([&](auto child) { return graph.prepare_relation(child, columns, dependencies); }, operand);
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
