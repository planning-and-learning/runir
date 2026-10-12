#ifndef RUNIR_KR_DL_SEMANTICS_DISTANCE_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_DISTANCE_EVALUATION_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/distance/feature.hpp"
#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/heuristics/projection.hpp"
#include "runir/kr/dl/translations/ygg_query.hpp"

#include <span>
#include <vector>
#include <yggdrasil/database/semantics/min_distance.hpp>

/// n_distance over a DL edge query, evaluated by yggdrasil (ygg::database::MinDistance in full evaluation,
/// ygg::database::incremental::MinDistance in incremental evaluation; see their method overviews). runir translates the
/// edge query into a yggdrasil query whose input slots are its leaves, complements becoming antijoins
/// (translations/ygg_query.hpp), supplies the slots' relations in the current state, and for k > 1 query edges a
/// pattern database heuristic over a static relaxation of the edge query (heuristics/projection.hpp). k = 1 and role
/// edges search blindly.

namespace runir::kr::dl::semantics::detail
{

template<typename Edges, FamilyTag Family, typename C>
inline constexpr bool is_role_edges_v = std::same_as<Edges, ygg::View<ygg::Index<Constructor<Family, RoleTag>>, C>>;

/// The rows of an input slot in the current state.
template<tyr::TaskKind Kind, FamilyTag Family, StateEvaluationContextConcept<Family, Kind> Context, typename C>
QueryDenotationView evaluate_slot(const QuerySlot& slot, const C& constructors, Context& context)
{
    return visit_slot<Family>(slot, constructors, [&](auto expression) { return evaluate_relation<Kind>(expression, slot.columns, context); });
}

/// Translates a feature's edges and builds its min-distance evaluator; role edges are a relation of one slot.
template<typename Distance, FamilyTag Family, typename C, typename Edges>
DistanceFeature<Distance> make_distance_feature(const ygg::database::DistancePlan<QueryValues>& plan, Edges edges)
{
    if constexpr (is_role_edges_v<Edges, Family, C>)
    {
        const auto columns = plan.edge_columns().span();
        return { { QuerySlot { QuerySlot::Kind::ROLE, ygg::uint_t(edges.get_index()), { columns.begin(), columns.end() }, edges.is_static() } },
                 Distance(plan, edges.is_static()) };
    }
    else
    {
        auto translator = dl::detail::QueryTranslator<Family, C>(true);
        const auto query = translator.translate(edges);
        auto static_slots = std::vector<bool>();
        for (const auto& slot : translator.get_slots())
            static_slots.push_back(slot.is_static);
        return { translator.get_slots(), Distance(query, plan, static_slots) };
    }
}

/// The heuristic of a feature: a pattern database heuristic for k > 1 query edges, otherwise blind;
/// `leaves(index)` returns a static leaf's rows.
template<FamilyTag Family, typename C, typename Edges, typename Leaves>
heuristics::ProjectionHeuristic
make_heuristic(const ygg::database::DistancePlan<QueryValues>& plan, Edges edges, ygg::uint_t num_objects, Leaves leaves)
{
    if constexpr (!is_role_edges_v<Edges, Family, C>)
        if (plan.arity() > 1)
            return heuristics::make_projection_heuristic<Family>(edges, plan, num_objects, std::move(leaves));
    return {};
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
    return ygg::visit(
        [&]<typename Edges>(Edges edges) -> ygg::uint_t
        {
            auto& feature = context.get_workspace().get_distance_feature(
                context.get_state().get_formalism_repository(),
                constructor.get_context(),
                ygg::uint_t(constructor.get_index()),
                [&]
                {
                    const auto leaves = [&](ygg::uint_t leaf)
                    { return evaluate<Kind>(ygg::make_view(ygg::Index<Query<Family>>(leaf), edges.get_context()), children); };
                    return FullDistanceFeature { make_distance_feature<ygg::database::MinDistance<QueryValues, heuristics::ProjectionHeuristic>, Family, C>(plan, edges),
                                                 make_heuristic<Family, C>(plan, edges, detail::num_objects<Kind, Family>(context), leaves) };
                });
            feature.heuristic.update(targets);
            const auto input = [&](size_t slot) { return evaluate_slot<Kind, Family>(feature.edges.slots[slot], edges.get_context(), children); };
            return feature.edges.distance.min_distance(input, sources, targets, feature.heuristic);
        },
        constructor.get_mid());
}

}  // namespace runir::kr::dl::semantics::detail

#endif
