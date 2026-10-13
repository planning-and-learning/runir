#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DISTANCE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DISTANCE_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>
#include <yggdrasil/database/semantics/incremental/distance.hpp>
#include <yggdrasil/database/semantics/incremental/min_distance.hpp>

namespace runir::kr::dl::semantics::incremental::detail
{

/// Distances over role edges: every argument is a relation in the graph (a query, or a concept or role lifted by the
/// graph), and yggdrasil maintains the minimum distance over the materialized edges (Ramalingam & Reps).
template<FamilyTag Family, tyr::TaskKind Kind>
struct RoleEdgeDistanceEvaluator
{
    QueryEvaluationIndex sources;
    QueryEvaluationIndex edges;
    QueryEvaluationIndex targets;
    ygg::database::incremental::SourceMinDistance<QueryValues> operation;
    DenotationState<NumericalTag> value;

    template<typename C>
    RoleEdgeDistanceEvaluator(ygg::View<ygg::Index<FamilyNumerical<Family, DistanceTag>>, C> expression,
                              GraphBuilder<Family, Kind>& graph,
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
                                        GraphBuilder<Family, Kind>& graph,
                                        std::vector<ygg::uint_t>& dependencies)
    {
        return ygg::visit([&](auto child) { return graph.prepare_relation(child, columns, dependencies); }, operand);
    }
};

/// Distances over query edges: the graph maintains the arguments and the edge query's input slots, and
/// ygg::database::incremental::MinDistance evaluates the edges on demand from them, so the edge relation, which for
/// pair vertices grows with |vertices| x degree, is never materialized.
template<FamilyTag Family, tyr::TaskKind Kind>
struct QueryEdgeDistanceEvaluator
{
    using C = ConstructorRepositoryFor<Family>;
    using Edges = ygg::View<ygg::Index<Query<Family>>, C>;
    using Distance = ygg::database::incremental::MinDistance<QueryValues, heuristics::ProjectionHeuristic>;

    ygg::View<ygg::Index<FamilyNumerical<Family, DistanceTag>>, C> expression;
    QueryEvaluationIndex sources;
    QueryEvaluationIndex targets;
    DistanceFeature<Distance> feature;
    std::vector<QueryEvaluationIndex> slots;  ///< The graph node of each input slot.
    std::optional<heuristics::ProjectionHeuristic> heuristic;  ///< Built at the first initialize(), from the bound leaves.
    DenotationState<NumericalTag> value;
    std::vector<bool> slot_changed;  ///< Per slot, whether update()'s delta changed it; kept to avoid allocations.

    QueryEdgeDistanceEvaluator(ygg::View<ygg::Index<FamilyNumerical<Family, DistanceTag>>, C> expression_,
                               GraphBuilder<Family, Kind>& graph,
                               std::vector<ygg::uint_t>& dependencies) :
        expression(expression_),
        sources(prepare(expression_.get_lhs(), expression_.get_data().plan.source_columns().span(), graph, dependencies)),
        targets(prepare(expression_.get_rhs(), expression_.get_data().plan.target_columns().span(), graph, dependencies)),
        feature(semantics::detail::make_distance_feature<Distance, Family, C>(expression_.get_data().plan, edges()))
    {
        for (const auto& slot : feature.slots)
            slots.push_back(visit_slot<Family>(slot, edges().get_context(), [&](auto argument) { return graph.prepare_relation(argument, slot.columns, dependencies); }));
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
    {
        feature.distance.bind(input(graph));
        if (!heuristic)
            heuristic = semantics::detail::make_heuristic(feature,
                                                          expression.get_data().plan,
                                                          semantics::detail::num_objects<Kind, Family>(context),
                                                          [&](size_t slot) -> const ygg::Builder<ygg::database::Relation<QueryValues>>& { return feature.distance.input(slot); });
        set_targets(graph);
        evaluate(graph);
    }
    /// The children's deltas are exact (IncrementalEvaluation): they are the slots' changes, unchanged arguments and
    /// slots leave the distance unchanged, and unchanged targets leave the search and the heuristic's tables valid.
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<QueryValues>&)
    {
        slot_changed.clear();
        for (const auto slot : slots)
            slot_changed.push_back(modified(graph, slot));
        const auto targets_changed = modified(graph, targets);
        if (!targets_changed && !modified(graph, sources) && std::ranges::none_of(slot_changed, std::identity {}))
        {
            value.clear_delta();
            return;
        }
        feature.distance.update([&](size_t slot) -> const auto& { return graph.change(slots[slot]); }, slot_changed);
        if (targets_changed)
            set_targets(graph);
        evaluate(graph);
    }

private:
    Edges edges() const { return expression.get_mid().template get<ygg::Index<Query<Family>>>(); }
    auto input(const EvaluationGraph<Family, Kind>& graph) const
    {
        return [this, &graph](size_t slot) -> const ygg::Builder<ygg::database::Relation<QueryValues>>& { return graph.result(slots[slot]); };
    }
    static bool modified(const EvaluationGraph<Family, Kind>& graph, QueryEvaluationIndex index)
    {
        return !graph.change(index).added.empty() || !graph.change(index).removed.empty();
    }
    void set_targets(const EvaluationGraph<Family, Kind>& graph)
    {
        feature.distance.set_targets(graph.result(targets));
        heuristic->update(graph.result(targets));
    }
    void evaluate(const EvaluationGraph<Family, Kind>& graph) { value.set(feature.distance.min_distance(graph.result(sources), *heuristic)); }
    template<typename Operand>
    static QueryEvaluationIndex prepare(Operand operand,
                                        std::span<const ygg::database::ColumnLayout> columns,
                                        GraphBuilder<Family, Kind>& graph,
                                        std::vector<ygg::uint_t>& dependencies)
    {
        return ygg::visit([&](auto child) { return graph.prepare_relation(child, columns, dependencies); }, operand);
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
