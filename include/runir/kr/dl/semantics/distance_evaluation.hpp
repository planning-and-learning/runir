#ifndef RUNIR_KR_DL_SEMANTICS_DISTANCE_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_DISTANCE_EVALUATION_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/distance/feature.hpp"
#include "runir/kr/dl/semantics/evaluation_decl.hpp"
#include "runir/kr/dl/semantics/heuristics/projection.hpp"
#include "runir/kr/dl/translations/ygg_query.hpp"

#include <cstddef>
#include <runir/kr/database/semantics/min_distance.hpp>
#include <runir/kr/database/semantics/relation_view.hpp>
#include <span>
#include <urd/repr/values/dense_set.hpp>
#include <urd/store.hpp>

/// n_distance over a DL edge query, evaluated by yggdrasil (runir::kr::database::MinDistance in full evaluation,
/// runir::kr::database::incremental::MinDistance in incremental evaluation; see their method overviews). runir translates the
/// edge query into a yggdrasil query whose input slots are its leaves, complements becoming antijoins
/// (translations/ygg_query.hpp), supplies the slots' relations in the current state, and for k > 1 query edges a
/// pattern database heuristic over a static relaxation of the edge query (heuristics/projection.hpp). k = 1 and role
/// edges search blindly.

namespace runir::kr::dl::semantics::detail
{

/// The rows of an input slot in the current state.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context>
QueryDenotationView evaluate_slot(const QuerySlot& slot, const ConstructorRepository& constructors, Context& context)
{
    return visit_slot<>(slot, constructors, [&](auto expression) { return evaluate_relation<Kind>(expression, slot.columns, context); });
}

/// Builds the min-distance evaluator of a feature with role edges, a relation of one slot.
template<typename Distance>
DistanceFeature<Distance>
make_distance_feature(ygg::FreeScratchRelations& scratch, const urd::data<runir::kr::database::DistancePlan>& plan, ConstructorView<RoleTag> edges)
{
    const auto& columns = plan.edge_columns;
    return { { QuerySlot { QuerySlot::Kind::ROLE, static_cast<ygg::uint_t>(edges.index().value), { columns.begin(), columns.end() }, edges.get_is_static() } },
             Distance(scratch, plan, edges.get_is_static()),
             std::nullopt };
}

/// Translates a feature's query edges and builds its min-distance evaluator.
template<typename Distance>
DistanceFeature<Distance> make_distance_feature(ygg::FreeScratchRelations& scratch, const urd::data<runir::kr::database::DistancePlan>& plan, QueryView<> edges)
{
    auto translator = dl::detail::QueryTranslator(true);
    const auto query = translator.translate(edges);
    auto static_slots = urd::Store<urd::set<size_t>, urd::dense>();
    for (size_t slot = 0; slot < translator.get_slots().size(); ++slot)
        if (translator.get_slots()[slot].is_static)
            static_slots.insert(slot);
    auto abstraction = arity(plan) > 1 ? heuristics::abstract_edges(translator, query, plan) : std::nullopt;
    return { translator.get_slots(), Distance(scratch, query, plan, static_slots), std::move(abstraction) };
}

/// The heuristic of a feature: a pattern database heuristic if it has an abstraction, otherwise blind;
/// `slot_rows(slot)` returns a static slot's rows.
template<typename Distance, runir::kr::database::SlotRelations SlotRows>
heuristics::ProjectionHeuristic make_heuristic(ygg::FreeScratchRelations& scratch,
                                               const DistanceFeature<Distance>& feature,
                                               const urd::data<runir::kr::database::DistancePlan>& plan,
                                               ygg::uint_t num_objects,
                                               const SlotRows& slot_rows)
{
    if (!feature.abstraction)
        return {};
    return heuristics::make_projection_heuristic(scratch, *feature.abstraction, plan, num_objects, slot_rows);
}

/// Minimum distance from any source to any target, or uint_t's maximum if none is reachable, from the state alone
/// (full evaluation): the arguments and the edge query's slots are evaluated in the state.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context>
ygg::uint_t on_demand_distance(NumericalView<DistanceTag> constructor, Context& context)
{
    const auto& plan = constructor.get_plan();
    auto children = context.child_context();
    const auto relation = [&](auto argument, auto columns)
    { return std::visit([&](auto child) { return evaluate_relation<Kind>(child, columns, children); }, argument); };
    const auto sources = relation(constructor.get_sources(), plan.source_columns);
    const auto targets = relation(constructor.get_targets(), plan.target_columns);
    return std::visit(
        [&]<typename Edges>(Edges edges) -> ygg::uint_t
        {
            auto& feature = context.get_workspace().get_distance_feature(
                context.get_state().get_formalism_repository(),
                constructor.context(),
                static_cast<ygg::uint_t>(constructor.index().value),
                [&]
                {
                    auto& scratch = context.get_workspace().get_scratch();
                    auto edge_feature = make_distance_feature<runir::kr::database::MinDistance<heuristics::ProjectionHeuristic>>(scratch, plan, edges);
                    auto heuristic = make_heuristic(scratch,
                                                    edge_feature,
                                                    plan,
                                                    detail::num_objects<Kind>(context),
                                                    [&](size_t slot) { return evaluate_slot<Kind>(edge_feature.slots[slot], edges.context(), children); });
                    return FullDistanceFeature { std::move(edge_feature), std::move(heuristic) };
                });
            feature.heuristic.update(targets);
            const auto input = [&](size_t slot) { return evaluate_slot<Kind>(feature.edges.slots[slot], edges.context(), children); };
            return feature.edges.distance.min_distance(input, sources, targets, feature.heuristic);
        },
        constructor.get_edges());
}

}  // namespace runir::kr::dl::semantics::detail

#endif
