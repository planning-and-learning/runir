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
#include <yggdrasil/database/semantics/relation_view.hpp>

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
template<tyr::TaskKind Kind, FamilyTag Family, StateEvaluationContextConcept<Family, Kind> Context>
QueryDenotationView evaluate_slot(const QuerySlot& slot, const ConstructorRepositoryFor<Family>& constructors, Context& context)
{
    return visit_slot<Family>(slot, constructors, [&](auto expression) { return evaluate_relation<Kind>(expression, slot.columns, context); });
}

/// Builds the min-distance evaluator of a feature with role edges, a relation of one slot.
template<typename Distance, FamilyTag Family>
DistanceFeature<Distance> make_distance_feature(const ygg::database::DistancePlan<QueryValues>& plan, FamilyConstructorView<Family, RoleTag> edges)
{
    const auto columns = plan.edge_columns().span();
    return { { QuerySlot { QuerySlot::Kind::ROLE, ygg::uint_t(edges.get_index()), { columns.begin(), columns.end() }, edges.is_static() } },
             Distance(plan, edges.is_static()),
             std::nullopt };
}

/// Translates a feature's query edges and builds its min-distance evaluator.
template<typename Distance, FamilyTag Family>
DistanceFeature<Distance> make_distance_feature(const ygg::database::DistancePlan<QueryValues>& plan, FamilyQueryView<Family> edges)
{
    auto translator = dl::detail::QueryTranslator<Family>(true);
    const auto query = translator.translate(edges);
    auto static_slots = std::vector<bool>();
    for (const auto& slot : translator.get_slots())
        static_slots.push_back(slot.is_static);
    auto abstraction = plan.arity() > 1 ? heuristics::abstract_edges(translator, query, plan) : std::nullopt;
    return { translator.get_slots(), Distance(query, plan, static_slots), std::move(abstraction) };
}

/// The heuristic of a feature: a pattern database heuristic if it has an abstraction, otherwise blind;
/// `slot_rows(slot)` returns a static slot's rows.
template<typename Distance, ygg::database::SlotRelations<QueryValues> SlotRows>
heuristics::ProjectionHeuristic
make_heuristic(const DistanceFeature<Distance>& feature, const ygg::database::DistancePlan<QueryValues>& plan, ygg::uint_t num_objects, const SlotRows& slot_rows)
{
    if (!feature.abstraction)
        return {};
    return heuristics::make_projection_heuristic(*feature.abstraction, plan, num_objects, slot_rows);
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
                    auto feature = make_distance_feature<ygg::database::MinDistance<QueryValues, heuristics::ProjectionHeuristic>, Family>(plan, edges);
                    auto heuristic = make_heuristic(feature,
                                                    plan,
                                                    detail::num_objects<Kind, Family>(context),
                                                    [&](size_t slot) { return evaluate_slot<Kind, Family>(feature.slots[slot], edges.get_context(), children); });
                    return FullDistanceFeature { std::move(feature), std::move(heuristic) };
                });
            feature.heuristic.update(targets);
            const auto input = [&](size_t slot) { return evaluate_slot<Kind, Family>(feature.edges.slots[slot], edges.get_context(), children); };
            return feature.edges.distance.min_distance(input, sources, targets, feature.heuristic);
        },
        constructor.get_mid());
}

}  // namespace runir::kr::dl::semantics::detail

#endif
