#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_MUST_PASS_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_MUST_PASS_HPP_

#include "runir/kr/dl/semantics/distance_evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/evaluators/query.hpp"
#include "runir/kr/dl/semantics/state_evaluation.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <span>
#include <vector>

namespace runir::kr::dl::semantics::incremental::detail
{

/// q_must_pass: the graph maintains the sources, the targets, the probes and the edge query's input slots, and
/// ygg::database::MustPass explores the edges on demand from them. The result is recomputed when one of them changed
/// and reported as its exact difference to the previous result; otherwise it is unchanged at no cost, which is the
/// common case for the bindings of a Choose rule when the query does not read the bound register.
template<tyr::TaskKind Kind>
struct QueryMustPassEvaluator : QueryValue
{
    QueryEvaluationIndex sources;
    QueryEvaluationIndex targets;
    QueryEvaluationIndex probes;
    MustPassFeature feature;
    std::vector<QueryEvaluationIndex> slots;         ///< The graph node of each input slot.
    ygg::Builder<ygg::database::Relation> computed;  ///< The recomputed result; kept to avoid allocations.
    std::vector<std::byte> removed;                  ///< Rows of the previous result that are gone.

    QueryMustPassEvaluator(QueryView<QueryMustPassTag> expression, GraphBuilder<Kind>& graph, std::vector<ygg::uint_t>& dependencies) :
        QueryValue(expression.get_schema().span()),
        sources(graph.prepare(expression.get_sources(), dependencies)),
        targets(graph.prepare(expression.get_targets(), dependencies)),
        probes(graph.prepare(expression.get_probes(), dependencies)),
        feature(semantics::detail::make_must_pass_feature(expression.get_data().plan, expression.get_edges())),
        computed(expression.get_schema().span())
    {
        const auto edges = expression.get_edges();
        for (const auto& slot : feature.slots)
            slots.push_back(
                visit_slot<>(slot, edges.get_context(), [&](auto argument) { return graph.prepare_relation(argument, slot.columns, dependencies); }));
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
    void update(EvaluationGraph<Kind>& graph, const Delta&, ygg::database::Workspace&)
    {
        this->delta.clear();
        if (!modified(graph, sources) && !modified(graph, targets) && !modified(graph, probes)
            && std::ranges::none_of(slots, [&](auto slot) { return modified(graph, slot); }))
            return;
        computed.clear();
        compute(graph, computed);
        const auto row_size = this->result.columns().row_size();
        removed.clear();
        for (size_t r = 0; r < this->result.size(); ++r)
            if (!computed.contains(this->result.row(r)))
                removed.insert(removed.end(), this->result.row(r).begin(), this->result.row(r).end());
        for (size_t offset = 0; offset < removed.size(); offset += row_size)
            this->set(std::span<const std::byte>(removed).subspan(offset, row_size), false);
        for (size_t r = 0; r < computed.size(); ++r)
            this->set(computed.row(r), true);
    }

private:
    ygg::uint_t num_objects = 0;

    void compute(EvaluationGraph<Kind>& graph, ygg::Builder<ygg::database::Relation>& out)
    {
        auto& heuristic = semantics::detail::must_pass_heuristic(feature, num_objects, input(graph), graph.result(targets));
        feature.must_pass.must_pass(input(graph), graph.result(sources), graph.result(targets), graph.result(probes), out, heuristic);
    }
    auto input(const EvaluationGraph<Kind>& graph) const
    {
        return [this, &graph](size_t slot) -> const ygg::Builder<ygg::database::Relation>& { return graph.result(slots[slot]); };
    }
    static bool modified(const EvaluationGraph<Kind>& graph, QueryEvaluationIndex index)
    { return !graph.change(index).added.empty() || !graph.change(index).removed.empty(); }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
