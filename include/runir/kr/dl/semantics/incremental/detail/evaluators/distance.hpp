#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DISTANCE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_EVALUATORS_DISTANCE_HPP_

#include "runir/kr/dl/semantics/ext/evaluation.hpp"
#include "runir/kr/dl/semantics/incremental/declarations.hpp"
#include "runir/kr/dl/semantics/incremental/delta.hpp"
#include "runir/kr/dl/semantics/incremental/detail/denotation_state.hpp"

#include <memory>
#include <span>
#include <stdexcept>
#include <vector>
#include <yggdrasil/database/semantics/incremental/distance.hpp>
#include <yggdrasil/database/semantics/incremental/min_distance.hpp>

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
    ygg::database::incremental::SourceMinDistance<QueryValues> operation;
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

/// Distances over query edges: the graph maintains the arguments and the edge query's slots, and
/// ygg::database::incremental::MinDistance evaluates the edges on demand from them, so the edge relation, which for
/// pair vertices grows with |vertices| x degree, is never materialized.
template<FamilyTag Family, tyr::TaskKind Kind>
struct GuidedDistanceEvaluator
{
    using Feature = DistanceFeature<ygg::database::incremental::MinDistance<QueryValues, heuristics::ProjectionHeuristic>, heuristics::ProjectionHeuristic>;

    ygg::View<ygg::Index<FamilyNumerical<Family, DistanceTag>>, ConstructorRepositoryFor<Family>> expression;
    QueryEvaluationIndex sources;
    QueryEvaluationIndex targets;
    std::vector<QueryEvaluationIndex> slots;
    Feature feature;
    DenotationState<NumericalTag> value;

    GuidedDistanceEvaluator(ygg::View<ygg::Index<FamilyNumerical<Family, DistanceTag>>, ConstructorRepositoryFor<Family>> expression_,
                            EvaluationGraph<Family, Kind>& graph,
                            std::vector<ygg::uint_t>& dependencies) :
        expression(expression_),
        sources(prepare(expression_.get_lhs(), expression_.get_data().plan.source_columns().span(), graph, dependencies)),
        targets(prepare(expression_.get_rhs(), expression_.get_data().plan.target_columns().span(), graph, dependencies))
    {
        using C = ConstructorRepositoryFor<Family>;
        const auto& plan = expression.get_data().plan;
        ygg::visit(
            [&](auto edges)
            {
                semantics::detail::prepare_distance<Family, C>(feature, plan, edges);
                for (const auto& slot : feature.slots)
                    slots.push_back(prepare_slot(slot, edges, plan.edge_columns().span(), graph, dependencies));
            },
            expression.get_mid());
    }
    template<StateEvaluationContextConcept<Family, Kind> Context>
    void initialize(EvaluationGraph<Family, Kind>& graph, Context& context)
    {
        feature.distance->bind(input(graph));
        using C = ConstructorRepositoryFor<Family>;
        const auto& plan = expression.get_data().plan;
        ygg::visit(
            [&](auto edges)
            {
                semantics::detail::prepare_heuristic<Family, C>(feature,
                                                                plan,
                                                                edges,
                                                                semantics::detail::num_objects<Kind, Family>(context),
                                                                [&](ygg::uint_t leaf) -> const ygg::Builder<ygg::database::Relation<QueryValues>>& { return leaf_rows(leaf); });
            },
            expression.get_mid());
        evaluate(graph);
    }
    void update(EvaluationGraph<Family, Kind>& graph, const Delta<Family>&, ygg::database::Workspace<QueryValues>&)
    {
        auto changed = std::vector<bool>();
        for (const auto slot : slots)
            changed.push_back(!graph.change(slot).added.empty() || !graph.change(slot).removed.empty());
        feature.distance->bind(input(graph), changed);
        evaluate(graph);
    }

private:
    auto input(const EvaluationGraph<Family, Kind>& graph) const
    {
        return [this, &graph](size_t slot) -> const ygg::Builder<ygg::database::Relation<QueryValues>>& { return graph.result(slots[slot]); };
    }
    /// A leaf's rows as bound in the min-distance evaluator.
    const ygg::Builder<ygg::database::Relation<QueryValues>>& leaf_rows(ygg::uint_t leaf) const
    {
        for (size_t slot = 0; slot < feature.slots.size(); ++slot)
            if (!feature.slots[slot].complement && feature.slots[slot].leaf == leaf)
                return feature.distance->input(slot);
        throw std::logic_error("Distance: relaxed leaf without an input slot.");
    }
    void evaluate(const EvaluationGraph<Family, Kind>& graph)
    {
        feature.heuristic.update(graph.result(targets));
        value.set(feature.distance->min_distance(graph.result(sources), graph.result(targets), feature.heuristic));
    }
    template<typename Operand>
    static QueryEvaluationIndex prepare(Operand operand,
                                        std::span<const ygg::database::ColumnLayout> columns,
                                        EvaluationGraph<Family, Kind>& graph,
                                        std::vector<ygg::uint_t>& dependencies)
    {
        return ygg::visit([&](auto child) { return graph.prepare_relation(child, columns, dependencies); }, operand);
    }
    /// A slot's relation in the graph: the leaf, the argument of a complemented leaf, or the role edges.
    template<typename Edges>
    static QueryEvaluationIndex prepare_slot(const QuerySlot& slot,
                                             Edges edges,
                                             std::span<const ygg::database::ColumnLayout> edge_columns,
                                             EvaluationGraph<Family, Kind>& graph,
                                             std::vector<ygg::uint_t>& dependencies)
    {
        using C = ConstructorRepositoryFor<Family>;
        if constexpr (std::same_as<Edges, ygg::View<ygg::Index<Constructor<Family, RoleTag>>, C>>)
            return graph.prepare_relation(edges, edge_columns, dependencies);
        else
        {
            const auto leaf = ygg::make_view(ygg::Index<Query<Family>>(slot.leaf), edges.get_context());
            if (!slot.complement)
                return graph.prepare_relation(leaf, {}, dependencies);
            return ygg::visit(
                [&]<typename Tag>(ygg::View<ygg::Index<Query<Family, Tag>>, C> concrete) -> QueryEvaluationIndex
                {
                    if constexpr (std::same_as<Tag, QueryRoleTag> || std::same_as<Tag, QueryConceptTag>)
                        return ygg::visit(
                            [&]<typename Complement>(Complement complement) -> QueryEvaluationIndex
                            {
                                if constexpr (dl::detail::is_complement_role_view_v<Complement> || dl::detail::is_negation_concept_view_v<Complement>)
                                    return graph.prepare_relation(complement.get_arg(), concrete.get_schema().span(), dependencies);
                                else
                                    throw std::logic_error("Distance: complement slot without a complement.");
                            },
                            concrete.get_arg().get_variant());
                    else
                        throw std::logic_error("Distance: complement slot without a role or concept leaf.");
                },
                leaf.get_variant());
        }
    }
};

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
