#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DATALOG_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DATALOG_HPP_

#include "runir/kr/dl/semantics/datalog_feature.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/query.hpp"
#include "runir/kr/dl/semantics/state_evaluation.hpp"

#include <ranges>
#include <span>
#include <vector>

namespace runir::kr::dl::semantics::incremental::detail
{

/// A Datalog program, as a node without a query expression that the program's q_head nodes depend on: it maintains the
/// program's heads (a DatalogFeature, as in full evaluation) from its input queries, initialized from their results and
/// updated from their exact changes. Its own result has no columns; heads read the feature.
template<tyr::TaskKind Kind>
struct QueryProgramEvaluator : QueryValue
{
    DatalogProgramView program;
    std::vector<QueryEvaluationIndex> inputs;  ///< The graph node of each input slot.
    DatalogFeature feature;

    QueryProgramEvaluator(DatalogProgramView program_, GraphBuilder<Kind>& graph, std::vector<ygg::uint_t>& dependencies) :
        QueryValue(std::span<const ygg::database::ColumnLayout>()),
        program(program_),
        feature(make_datalog_feature(program_))
    {
        for (const auto input : program.get_inputs())
            inputs.push_back(graph.prepare(input, dependencies));
    }

    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        feature.evaluator.initialize(inputs | std::views::transform([&](QueryEvaluationIndex input) -> const auto& { return graph.result(input); }));
    }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
    {
        feature.evaluator.update(inputs | std::views::transform([&](QueryEvaluationIndex input) { return graph.change(input).change(); }));
    }
};

/// q_head: the rows of a head of its program's node, in the query's columns (the same uint_t rows, other labels).
template<tyr::TaskKind Kind>
struct QueryHeadEvaluator : QueryValue
{
    QueryEvaluationIndex program;
    ygg::Index<ygg::database::datalog::Relation> relation;

    QueryHeadEvaluator(QueryView<QueryHeadTag> expression, GraphBuilder<Kind>& graph, std::vector<ygg::uint_t>& dependencies) :
        QueryValue(expression.get_schema().span()),
        program(graph.prepare_program(expression.get_program(), dependencies)),
        relation(expression.get_data().relation)
    {
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        auto& output_rows = this->clear();
        const auto& feature = graph.program(program).feature;
        const auto& head = feature.evaluator.get_result(feature.output(relation));
        for (size_t i = 0; i < head.size(); ++i)
            output_rows.insert(head.row(i));
    }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
    {
        this->delta.clear();
        const auto& feature = graph.program(program).feature;
        const auto& change = feature.evaluator.get_delta(feature.output(relation));
        for (size_t i = 0; i < change.removed.size(); ++i)
            this->set(change.removed.row(i), false);
        for (size_t i = 0; i < change.added.size(); ++i)
            this->set(change.added.row(i), true);
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
