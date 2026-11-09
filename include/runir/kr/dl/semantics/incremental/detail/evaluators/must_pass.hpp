#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_MUST_PASS_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_MUST_PASS_HPP_

#include "runir/kr/dl/semantics/distance_evaluation.hpp"
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/query.hpp"
#include "runir/kr/dl/datas.hpp"
#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/state_evaluation_context.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <span>
#include <vector>

namespace runir::kr::dl::semantics::incremental::detail
{

/// q_must_pass: the graph maintains the sources, the targets, the probes and the edge query's input slots, and
/// tyr::datalog::database::MustPass explores the edges on demand from them. The result is recomputed when one of them changed
/// and reported as its exact difference to the previous result; otherwise it is unchanged at no cost, which is the
/// common case for the bindings of a Choose rule when the query does not read the bound register.
template<tyr::TaskKind Kind>
struct QueryMustPassEvaluator : QueryValue
{
    QueryMustPassEvaluator(QueryView<QueryMustPassTag> expression, GraphBuilder<Kind>& graph, urd::Store<urd::sequence<ygg::uint_t>>& dependencies) :
        QueryValue(graph.layer(), expression.get_schema()),
        sources(graph.prepare(expression.get_sources(), dependencies)),
        targets(graph.prepare(expression.get_targets(), dependencies)),
        probes(graph.prepare(expression.get_probes(), dependencies)),
        feature(semantics::detail::make_must_pass_feature(graph.layer(), expression.data().plan, expression.get_edges())),
        computed(expression.get_schema())
    {
        const auto edges = expression.get_edges();
        for (const auto& slot : feature.slots)
            slots.push_back(
                visit_slot<>(slot, edges.context(), [&](auto argument) { return graph.prepare_relation(argument, slot.columns, dependencies); }));
    }
    template<StateEvaluationContextConcept<Kind> Context>
    void initialize(EvaluationGraph<Kind>& graph, Context& context)
    {
        num_objects = semantics::detail::num_objects<Kind>(context);
        auto& output = this->clear();
        compute(graph, output);
    }
    /// The children's deltas are exact (IncrementalEvaluation), so unchanged sources, targets and slots leave the
    /// result unchanged.
    void update(EvaluationGraph<Kind>& graph, const Delta&, tyr::datalog::database::Workspace&)
    {
        this->delta.clear();
        if (!modified(graph, sources) && !modified(graph, targets) && !modified(graph, probes)
            && std::ranges::none_of(slots, [&](auto slot) { return modified(graph, slot); }))
            return;
        computed.clear();
        compute(graph, computed);
        const auto row_size = this->result.columns().size();
        const auto previous = this->get_result();
        removed.clear();
        for (size_t r = 0; r < previous.size(); ++r)
            if (const auto row = previous.row(r); !computed.contains(row))
            {
                const auto values = tyr::datalog::database::vertex_values(row, scratch);
                removed.insert(removed.end(), values.begin(), values.end());
            }
        for (size_t offset = 0; offset < removed.size(); offset += row_size)
            this->set(std::span<const ygg::Value>(removed).subspan(offset, row_size), false);
        const auto recomputed = this->layer->view(computed);
        for (size_t r = 0; r < recomputed.size(); ++r)
            this->set(recomputed.row(r).get_values(), true);
    }

private:
    QueryEvaluationIndex sources;
    QueryEvaluationIndex targets;
    QueryEvaluationIndex probes;
    MustPassFeature feature;
    urd::Store<urd::sequence<QueryEvaluationIndex>> slots;         ///< The graph node of each input slot.
    urd::builder<ygg::Relation<ygg::Schema>> computed;  ///< The recomputed result; kept to avoid allocations.
    urd::Store<urd::sequence<ygg::Value>> removed;                 ///< Rows of the previous result that are gone, their values.
    urd::Store<urd::sequence<ygg::Value>> scratch;                 ///< A row's values, as read.

    ygg::uint_t num_objects = 0;

    void compute(EvaluationGraph<Kind>& graph, urd::builder<ygg::Relation<ygg::Schema>>& out)
    {
        auto& heuristic = semantics::detail::must_pass_heuristic(graph.layer(), feature, num_objects, input(graph), graph.result(targets));
        feature.must_pass.must_pass(input(graph), graph.result(sources), graph.result(targets), graph.result(probes), out, heuristic);
    }
    auto input(const EvaluationGraph<Kind>& graph) const
    {
        return [this, &graph](size_t slot) { return graph.result(slots[slot]); };
    }
    static bool modified(const EvaluationGraph<Kind>& graph, QueryEvaluationIndex index)
    { return !graph.change(index).added.empty() || !graph.change(index).removed.empty(); }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
