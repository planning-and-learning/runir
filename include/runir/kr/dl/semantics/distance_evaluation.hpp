#ifndef RUNIR_KR_DL_SEMANTICS_DISTANCE_EVALUATION_HPP_
#define RUNIR_KR_DL_SEMANTICS_DISTANCE_EVALUATION_HPP_

#include "runir/kr/dl/query_view.hpp"
#include "runir/kr/dl/semantics/detail/evaluation_helpers.hpp"
#include "runir/kr/dl/semantics/distance/feature.hpp"
#include "runir/kr/dl/semantics/evaluation_decl.hpp"
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

/// The rows of an input slot in the current state.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context>
QueryDenotationView evaluate_slot(const QuerySlot& slot, const ConstructorRepository& constructors, Context& context)
{
    return visit_slot<>(slot, constructors, [&](auto expression) { return evaluate_relation<Kind>(expression, slot.columns, context); });
}

/// Builds the min-distance evaluator of a feature with role edges, a relation of one slot.
template<typename Distance>
DistanceFeature<Distance> make_distance_feature(const ygg::database::DistancePlan& plan, ConstructorView<RoleTag> edges)
{
    const auto columns = plan.edge_columns().span();
    return { { QuerySlot { QuerySlot::Kind::ROLE, static_cast<ygg::uint_t>(edges.get_index()), { columns.begin(), columns.end() }, edges.is_static() } },
             Distance(plan, edges.is_static()),
             std::nullopt };
}

/// Translates a feature's query edges and builds its min-distance evaluator.
template<typename Distance>
DistanceFeature<Distance> make_distance_feature(const ygg::database::DistancePlan& plan, QueryView<> edges)
{
    auto translator = dl::detail::QueryTranslator(true);
    const auto query = translator.translate(edges);
    auto static_slots = std::vector<bool>();
    for (const auto& slot : translator.get_slots())
        static_slots.push_back(slot.is_static);
    auto abstraction = plan.arity() > 1 ? heuristics::abstract_edges(translator, query, plan) : std::nullopt;
    return { translator.get_slots(), Distance(query, plan, static_slots), std::move(abstraction) };
}

/// The heuristic of a feature: a pattern database heuristic if it has an abstraction, otherwise blind;
/// `slot_rows(slot)` returns a static slot's rows.
template<typename Distance, ygg::database::SlotRelations SlotRows>
heuristics::ProjectionHeuristic
make_heuristic(const DistanceFeature<Distance>& feature, const ygg::database::DistancePlan& plan, ygg::uint_t num_objects, const SlotRows& slot_rows)
{
    if (!feature.abstraction)
        return {};
    return heuristics::make_projection_heuristic(*feature.abstraction, plan, num_objects, slot_rows);
}

/// Minimum distance from any source to any target, or uint_t's maximum if none is reachable, from the state alone
/// (full evaluation): the arguments and the edge query's slots are evaluated in the state.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context>
ygg::uint_t on_demand_distance(NumericalView<DistanceTag> constructor, Context& context)
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
                static_cast<ygg::uint_t>(constructor.get_index()),
                [&]
                {
                    auto edge_feature = make_distance_feature<ygg::database::MinDistance<heuristics::ProjectionHeuristic>>(plan, edges);
                    auto heuristic = make_heuristic(edge_feature,
                                                    plan,
                                                    detail::num_objects<Kind>(context),
                                                    [&](size_t slot) { return evaluate_slot<Kind>(edge_feature.slots[slot], edges.get_context(), children); });
                    return FullDistanceFeature { std::move(edge_feature), std::move(heuristic) };
                });
            feature.heuristic.update(targets);
            const auto input = [&](size_t slot) { return evaluate_slot<Kind>(feature.edges.slots[slot], edges.get_context(), children); };
            return feature.edges.distance.min_distance(input, sources, targets, feature.heuristic);
        },
        constructor.get_mid());
}

/// The vertex part of keyed columns, laid out on its own.
inline ygg::Builder<ygg::database::Columns> vertex_columns(std::span<const ygg::database::ColumnLayout> columns, size_t keys)
{
    auto result = ygg::Builder<ygg::database::Columns>();
    for (const auto& column : columns.subspan(keys))
        result.push_back(column.label, column.type);
    return result;
}

/// Translates a must-pass query's edges and builds its must-pass evaluator and the abstraction of its heuristic.
inline MustPassFeature make_must_pass_feature(const ygg::database::MustPassPlan& plan, QueryView<> edges)
{
    auto translator = dl::detail::QueryTranslator(true);
    const auto query = translator.translate(edges);
    auto static_slots = std::vector<bool>();
    for (const auto& slot : translator.get_slots())
        static_slots.push_back(slot.is_static);
    const auto columns = plan.edge_columns().span().subspan(plan.keys());
    auto abstraction = heuristics::abstract_edges(translator, query, columns.first(plan.arity()), columns.subspan(plan.arity()));
    return { translator.get_slots(), MustPassFeature::MustPass(query, plan, static_slots), std::move(abstraction), std::nullopt };
}

/// Builds a must-pass feature's heuristic at first use, from the static slots' rows (`slot_rows(slot)`), and sets its
/// targets. Without an abstraction, the search is blind.
template<ygg::database::SlotRelations SlotRows, ygg::database::RelationViewConcept Targets>
heuristics::KeyedProjectionHeuristic& must_pass_heuristic(MustPassFeature& feature, ygg::uint_t num_objects, const SlotRows& slot_rows, const Targets& targets)
{
    if (!feature.heuristic)
    {
        const auto& plan = feature.must_pass.plan();
        const auto vertices = vertex_columns(plan.source_columns().span(), plan.keys());
        const auto ends = vertex_columns(plan.target_columns().span(), plan.keys());
        const auto columns = plan.edge_columns().span().subspan(plan.keys());
        auto heuristic = feature.abstraction ? heuristics::make_projection_heuristic(*feature.abstraction,
                                                                                     columns.first(plan.arity()),
                                                                                     columns.subspan(plan.arity()),
                                                                                     vertices.span(),
                                                                                     ends.span(),
                                                                                     num_objects,
                                                                                     slot_rows) :
                                               heuristics::ProjectionHeuristic();
        feature.heuristic.emplace(std::move(heuristic), plan.key_size(), ends.span());
    }
    feature.heuristic->update(targets);
    return *feature.heuristic;
}

/// The must-pass probe objects of every key, from the state alone (full evaluation): sources, targets, probes and the
/// edge query's slots are evaluated in the state, and the edges are explored on demand.
template<tyr::TaskKind Kind, StateEvaluationContextConcept<Kind> Context, typename C>
void on_demand_must_pass(ygg::View<ygg::Index<Query<QueryMustPassTag>>, C> constructor, Context& context, ygg::Builder<ygg::database::Relation>& result)
{
    auto children = context.child_context();
    const auto sources = evaluate<Kind>(constructor.get_sources(), children);
    const auto targets = evaluate<Kind>(constructor.get_targets(), children);
    const auto probes = evaluate<Kind>(constructor.get_probes(), children);
    const auto edges = constructor.get_edges();
    auto& feature = context.get_workspace().get_must_pass_feature(context.get_state().get_formalism_repository(),
                                                                  constructor.get_context(),
                                                                  static_cast<ygg::uint_t>(constructor.get_index()),
                                                                  [&] { return make_must_pass_feature(constructor.get_data().plan, edges); });
    const auto input = [&](size_t slot) { return evaluate_slot<Kind>(feature.slots[slot], edges.get_context(), children); };
    auto& heuristic = must_pass_heuristic(feature, detail::num_objects<Kind>(context), input, targets);
    feature.must_pass.must_pass(input, sources, targets, probes, result, heuristic);
}

}  // namespace runir::kr::dl::semantics::detail

#endif
