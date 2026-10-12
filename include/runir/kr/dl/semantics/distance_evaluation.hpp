#ifndef RUNIR_KR_DL_SEMANTICS_DISTANCE_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_DISTANCE_EVALUATION_HPP_

#include "runir/kr/dl/complement.hpp"
#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/distance/feature.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/heuristics/projection.hpp"
#include "runir/kr/dl/semantics/heuristics/query_relaxation.hpp"
#include "runir/kr/dl/translations/ygg_query.hpp"

#include <span>
#include <stdexcept>
#include <vector>
#include <yggdrasil/database/semantics/min_distance.hpp>

/// n_distance over a DL edge query, evaluated by yggdrasil (ygg::database::MinDistance in full evaluation,
/// ygg::database::incremental::MinDistance in incremental evaluation; see their method overviews). runir lowers the
/// edge query into a yggdrasil query whose input slots are its leaves (translations/ygg_query.hpp), supplies the
/// slots' relations in the current state, and for k > 1 query edges a projection heuristic built from a static
/// relaxation of the edge query (heuristics/). k = 1 and role edges search blindly.

namespace runir::kr::dl::semantics::detail
{

/// The rows of an input slot in the current state: its leaf, or the argument of its complement.
template<tyr::TaskKind Kind, FamilyTag Family, StateEvaluationContextConcept<Family, Kind> Context, typename C>
QueryDenotationView evaluate_slot(const QuerySlot& slot, const C& constructors, Context& context)
{
    const auto leaf = ygg::make_view(ygg::Index<Query<Family>>(slot.leaf), constructors);
    if (!slot.complement)
        return evaluate<Kind>(leaf, context);
    return ygg::visit(
        [&]<typename Tag>(ygg::View<ygg::Index<Query<Family, Tag>>, C> concrete) -> QueryDenotationView
        {
            if constexpr (std::same_as<Tag, QueryRoleTag> || std::same_as<Tag, QueryConceptTag>)
                return ygg::visit(
                    [&]<typename Complement>(Complement complement) -> QueryDenotationView
                    {
                        if constexpr (dl::detail::is_complement_role_view_v<Complement> || dl::detail::is_negation_concept_view_v<Complement>)
                            return evaluate_relation<Kind>(complement.get_arg(), concrete.get_schema().span(), context);
                        else
                            throw std::logic_error("Distance: complement slot without a complement.");
                    },
                    concrete.get_arg().get_variant());
            else
                throw std::logic_error("Distance: complement slot without a role or concept leaf.");
        },
        leaf.get_variant());
}

template<typename Edges, FamilyTag Family, typename C>
inline constexpr bool is_role_edges_v = std::same_as<Edges, ygg::View<ygg::Index<Constructor<Family, RoleTag>>, C>>;

/// Lowers a feature's edges and builds its min-distance evaluator; role edges are a relation.
template<FamilyTag Family, typename C, typename Distance, typename H, typename Edges>
void prepare_distance(DistanceFeature<Distance, H>& feature, const ygg::database::DistancePlan<QueryValues>& plan, Edges edges)
{
    if constexpr (is_role_edges_v<Edges, Family, C>)
    {
        feature.slots.push_back(QuerySlot { 0, false, edges.is_static() });
        feature.distance.emplace(plan, edges.is_static());
    }
    else
    {
        auto lowering = dl::detail::EdgeLowering<Family, C>(feature.slots);
        const auto query = lowering.lower(edges);
        auto static_slots = std::vector<bool>();
        for (const auto& slot : feature.slots)
            static_slots.push_back(slot.is_static);
        feature.distance.emplace(query, plan, static_slots);
    }
}

/// Builds the projection heuristic once, for k > 1 query edges; `leaves(index)` returns a static leaf's rows.
template<FamilyTag Family, typename C, typename Distance, typename Edges, typename Leaves>
void prepare_heuristic(DistanceFeature<Distance, heuristics::ProjectionHeuristic>& feature,
                       const ygg::database::DistancePlan<QueryValues>& plan,
                       Edges edges,
                       ygg::uint_t num_objects,
                       Leaves leaves)
{
    if (feature.prepared)
        return;
    if constexpr (!is_role_edges_v<Edges, Family, C>)
        if (plan.arity() > 1)
            feature.heuristic = make_projection_heuristic<Family>(edges, plan, num_objects, std::move(leaves));
    feature.prepared = true;
}

/// Minimum distance from any source to any target, or uint_t's maximum if none is reachable, from the state alone
/// (full evaluation): the arguments and the edge query's slots are evaluated in the state.
template<tyr::TaskKind Kind, FamilyTag Family, StateEvaluationContextConcept<Family, Kind> Context, typename C>
ygg::uint_t on_demand_distance(ygg::View<ygg::Index<FamilyNumerical<Family, DistanceTag>>, C> constructor, Context& context)
{
    const auto& plan = constructor.get_data().plan;
    auto children = context.child_context();
    const auto relation = [&](auto argument, auto columns)
    { return ygg::visit([&](auto child) { return evaluate_relation<Kind>(child, columns.span(), children); }, argument); };
    const auto sources = relation(constructor.get_lhs(), plan.source_columns());
    const auto targets = relation(constructor.get_rhs(), plan.target_columns());
    auto& feature = context.get_workspace().get_distance_feature(&*context.get_state().get_formalism_repository(),
                                                                 &constructor.get_context(),
                                                                 ygg::uint_t(constructor.get_index()));
    return ygg::visit(
        [&]<typename Edges>(Edges edges) -> ygg::uint_t
        {
            if (!feature.distance)
                prepare_distance<Family, C>(feature, plan, edges);
            if constexpr (!is_role_edges_v<Edges, Family, C>)
                prepare_heuristic<Family, C>(feature,
                                             plan,
                                             edges,
                                             detail::num_objects<Kind, Family>(context),
                                             [&](ygg::uint_t leaf) { return evaluate<Kind>(ygg::make_view(ygg::Index<Query<Family>>(leaf), edges.get_context()), children); });
            feature.heuristic.update(targets);
            const auto input = [&](size_t slot) -> QueryDenotationView
            {
                if constexpr (is_role_edges_v<Edges, Family, C>)
                    return evaluate_relation<Kind>(edges, plan.edge_columns().span(), children);
                else
                    return evaluate_slot<Kind, Family>(feature.slots[slot], edges.get_context(), children);
            };
            return feature.distance->min_distance(input, sources, targets, feature.heuristic);
        },
        constructor.get_mid());
}

}  // namespace runir::kr::dl::semantics::detail

#endif
