#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DATALOG_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DATALOG_HPP_

#include "runir/kr/dl/semantics/datalog_feature.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/query.hpp"

#include <ranges>
#include <span>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

/// A Datalog program, as a node without a query expression that the program's q_head nodes depend on: it maintains the
/// program's heads (a DatalogFeature, as in full evaluation) from its input queries, initialized from their results and
/// updated from their exact changes. Its own result has no columns; heads read the feature.
template<tyr::TaskKind Kind>
struct QueryProgramEvaluator : QueryValue
{
    DatalogProgramView program;
    urd::Store<urd::sequence<QueryEvaluationIndex>> inputs;  ///< The graph node of each input slot.
    DatalogFeature feature;

    QueryProgramEvaluator(DatalogProgramView program_, GraphBuilder<Kind>& graph, urd::Store<urd::sequence<ygg::uint_t>>& dependencies) :
        QueryValue(graph.layer(), std::span<const ygg::ColumnLayout>()),
        program(program_),
        feature(make_datalog_feature(graph.layer(), program_))
    {
        for (const auto input : program.get_inputs())
            inputs.push_back(graph.prepare(input, dependencies));
    }

    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        feature.evaluator.initialize(inputs | std::views::transform([&](QueryEvaluationIndex input) { return graph.result(input); }));
    }
    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace&)
    {
        feature.evaluator.update(inputs | std::views::transform([&](QueryEvaluationIndex input) { return graph.change(input).change(); }));
    }
};

/// q_head: the rows of a head of its program's node, in the query's columns (the same values, other labels and types).
template<tyr::TaskKind Kind>
struct QueryHeadEvaluator : QueryValue
{
    QueryEvaluationIndex program;
    urd::index<tyr::datalog::database::datalog::Relation> relation;
    urd::Store<urd::sequence<ygg::Value>> values;  ///< A head row's values.

    QueryHeadEvaluator(const ygg::FreeRelationRepository& layer_,
                       QueryView<QueryHeadTag> expression,
                       GraphBuilder<Kind>& graph,
                       urd::Store<urd::sequence<ygg::uint_t>>& dependencies) :
        QueryValue(layer_, expression.get_schema()),
        program(graph.prepare_program(expression.get_program(), dependencies)),
        relation(expression.data().relation)
    {
    }
    /// A head row's values, which the query's columns hold.
    template<typename Row>
    std::span<const ygg::Value> row_values(const Row& row)
    {
        const auto stored = row.get_values();
        values.clear();
        for (size_t c = 0; c < stored.size(); ++c)
            values.push_back(stored.get(c));
        return values;
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        auto& rows = this->clear();
        const auto& feature = graph.program(program).feature;
        const auto head = feature.evaluator.get_result(feature.output(relation));
        for (size_t i = 0; i < head.size(); ++i)
            rows.insert(row_values(head.row(i)));
    }
    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace&)
    {
        this->delta.clear();
        const auto& feature = graph.program(program).feature;
        const auto [added, removed] = feature.evaluator.get_delta(feature.output(relation)).change();
        for (size_t i = 0; i < removed.size(); ++i)
            this->set(row_values(removed.row(i)), false);
        for (size_t i = 0; i < added.size(); ++i)
            this->set(row_values(added.row(i)), true);
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
