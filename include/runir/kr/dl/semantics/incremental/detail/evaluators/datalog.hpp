#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DATALOG_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DATALOG_HPP_

#include "runir/kr/dl/semantics/datalog_feature.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/query.hpp"
#include "runir/kr/dl/semantics/state_evaluation.hpp"

#include <algorithm>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>
#include <yggdrasil/database/datalog/evaluator.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

/// A Datalog program, as a node without a query expression that the program's q_head nodes depend on: it maintains
/// the requested heads with yggdrasil's datalog::Evaluator from its input queries, initialized from their results
/// and updated from their exact changes. Its own result is empty; heads read the evaluator.
template<tyr::TaskKind Kind>
struct QueryProgramEvaluator : QueryValue
{
    DatalogProgramView program;
    std::vector<QueryEvaluationIndex> inputs;                         ///< The graph node of each input slot.
    std::vector<ygg::Index<ygg::database::datalog::Relation>> heads;  ///< The requested heads, the evaluator's outputs.
    std::optional<ygg::database::datalog::Evaluator> evaluator;       ///< Built at the first evaluation, once all heads are known.

    QueryProgramEvaluator(DatalogProgramView program_, GraphBuilder<Kind>& graph, std::vector<ygg::uint_t>& dependencies) :
        QueryValue(std::span<const ygg::database::ColumnLayout>()),
        program(program_)
    {
        for (const auto input : program.get_inputs())
            inputs.push_back(graph.prepare(input, dependencies));
    }

    /// The output of a head, requested while the graph is built.
    size_t request(ygg::Index<ygg::database::datalog::Relation> relation)
    {
        if (const auto it = std::ranges::find(heads, relation); it != heads.end())
            return static_cast<size_t>(it - heads.begin());
        if (evaluator)
            throw std::logic_error("q_head: a head was requested after its program was evaluated.");
        heads.push_back(relation);
        return heads.size() - 1;
    }

    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        if (!evaluator)
        {
            auto views = std::vector<ygg::database::datalog::RelationView>();
            for (const auto head : heads)
                views.emplace_back(head, program.get_context().get_datalog_repository());
            evaluator.emplace(program.get_program(), views, semantics::static_slots(program));
        }
        evaluator->initialize(inputs | std::views::transform([&](QueryEvaluationIndex input) -> const auto& { return graph.result(input); }));
    }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
    {
        evaluator->update(inputs | std::views::transform([&](QueryEvaluationIndex input) { return graph.change(input).change(); }));
    }
};

/// q_head: the rows of a head of its program's node, in the query's columns (the same uint_t rows, other labels).
template<tyr::TaskKind Kind>
struct QueryHeadEvaluator : QueryValue
{
    QueryEvaluationIndex program;
    size_t output;

    QueryHeadEvaluator(QueryView<QueryHeadTag> expression, GraphBuilder<Kind>& graph, std::vector<ygg::uint_t>& dependencies) :
        QueryValue(expression.get_schema().span()),
        program(graph.prepare_program(expression.get_program(), dependencies)),
        output(graph.request_head(program, expression.get_data().relation))
    {
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context&)
    {
        auto& output_rows = this->clear();
        const auto& head = graph.program(program).evaluator->get_result(output);
        for (size_t i = 0; i < head.size(); ++i)
            output_rows.insert(head.row(i));
    }
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
    {
        this->delta.clear();
        const auto& change = graph.program(program).evaluator->get_delta(output);
        for (size_t i = 0; i < change.removed.size(); ++i)
            this->set(change.removed.row(i), false);
        for (size_t i = 0; i < change.added.size(); ++i)
            this->set(change.added.row(i), true);
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
