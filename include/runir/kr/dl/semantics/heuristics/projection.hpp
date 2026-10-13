#ifndef RUNIR_KR_DL_SEMANTICS_HEURISTICS_PROJECTION_HPP_
#define RUNIR_KR_DL_SEMANTICS_HEURISTICS_PROJECTION_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/translations/ygg_query.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <optional>
#include <span>
#include <utility>
#include <tyr/formalism/declarations.hpp>
#include <vector>
#include <yggdrasil/database/semantics/bound/query_evaluation.hpp>
#include <yggdrasil/database/semantics/search/concepts.hpp>
#include <yggdrasil/database/semantics/distance.hpp>
#include <yggdrasil/database/semantics/operations.hpp>
#include <yggdrasil/database/semantics/relation_view.hpp>
#include <yggdrasil/database/syntax/columns_builder.hpp>
#include <yggdrasil/database/syntax/copy.hpp>
#include <yggdrasil/database/syntax/query.hpp>
#include <yggdrasil/database/syntax/row.hpp>

namespace runir::kr::dl::semantics::heuristics
{

using Object = ygg::Index<tyr::formalism::Object>;
using Row = ygg::database::Row<QueryValues>;

/// The pattern database of one vertex column i, the pattern: the abstraction α_i(v) = v[i] maps the edges to a graph
/// over objects, and h is its breadth-first distance table to the abstract targets α_i(T). The abstract edges are not
/// stored: the breadth-first search evaluates their query with v[i] bound, per vertex, as the distance search evaluates
/// its edges.
struct PatternDatabase
{
    size_t pattern = 0;                                    ///< The vertex column i.
    ygg::database::bound::QueryEvaluator<QueryValues> edges;  ///< The abstract edges (u[i], v[i]), over static inputs only.
    size_t u_position = 0;                                 ///< The position of u[i] in the columns of edges.
    size_t v_position = 0;                                 ///< The position of v[i] in the columns of edges.
    size_t predecessors = 0;                               ///< The binding of edges with v[i] bound.
    std::vector<bool> targets;                             ///< α_i(T), for which h was computed.
    std::vector<ygg::uint_t> h;                            ///< Abstract distance to α_i(T); ygg::database::infinity if unreachable.

    /// Whether h has been computed, for some abstract targets.
    bool computed() const noexcept { return !h.empty(); }
    /// Whether h must be computed for `abstract_targets`: it never was, or it was for other abstract targets.
    bool needs_update(const std::vector<bool>& abstract_targets) const { return !computed() || targets != abstract_targets; }
};

/// h(v) = max_i h_i(α_i(v)) over single-column pattern databases (Culberson & Schaeffer, "Pattern Databases",
/// Computational Intelligence 1998; maximizing over several, Holte et al., "Maximizing over Multiple Pattern Databases
/// Speeds up Heuristic Search", AIJ 2006). The abstract edges come from the relaxation of the edge query with only its
/// static slots known (ygg::database::relax), projected onto (u[i], v[i]): a superset of the projected edges in every
/// state (EdgeAbstraction). Every edge u -> v thus
/// maps to an abstract edge, or to a self-loop when u[i] = v[i], so each h_i, and their maximum, is consistent in every
/// state; only the tables depend on the targets. Without pattern databases, h = 0 (blind search).
class ProjectionHeuristic
{
public:
    ProjectionHeuristic() = default;
    ProjectionHeuristic(std::vector<PatternDatabase> databases,
                        std::span<const ygg::database::ColumnLayout> vertex_columns,
                        std::span<const ygg::database::ColumnLayout> target_columns,
                        ygg::uint_t num_objects) :
        m_databases(std::move(databases)),
        m_vertex_columns(vertex_columns.begin(), vertex_columns.end()),
        m_target_columns(target_columns.begin(), target_columns.end()),
        m_num_objects(num_objects)
    {
    }

    /// Recomputes the tables whose abstract targets changed. Callers that know the targets are unchanged skip it, but
    /// update() must have been called once before estimate().
    template<ygg::database::RelationViewConcept<QueryValues> Targets>
    void update(const Targets& targets)
    {
        for (auto& database : m_databases)
            update_table(database, targets);
    }

    /// h(vertex); requires update() to have computed every table.
    ygg::uint_t estimate(std::span<const std::byte> vertex) const
    {
        ygg::uint_t value = 0;
        for (const auto& database : m_databases)
        {
            assert(database.computed());
            const auto entry = database.h[ygg::uint_t(Row(vertex, m_vertex_columns).get<Object>(database.pattern))];
            if (entry == ygg::database::infinity)
                return entry;
            value = std::max(value, entry);
        }
        return value;
    }

private:
    std::vector<PatternDatabase> m_databases;
    std::vector<ygg::database::ColumnLayout> m_vertex_columns;
    std::vector<ygg::database::ColumnLayout> m_target_columns;
    ygg::uint_t m_num_objects = 0;
    std::vector<bool> m_abstract_targets;  ///< Scratch of update_table(), kept with m_queue to avoid allocations.
    std::vector<ygg::uint_t> m_queue;
    std::vector<std::byte> m_vertex;  ///< The bound abstract vertex, kept to avoid allocations.

    /// A backward breadth-first search from α_i(T) in the abstract graph. The table is built aside and committed with
    /// its targets only when the search succeeds: a throw leaves computed() false, so the next update() rebuilds it.
    template<ygg::database::RelationViewConcept<QueryValues> Targets>
    void update_table(PatternDatabase& database, const Targets& targets)
    {
        m_abstract_targets.assign(m_num_objects, false);
        for (size_t r = 0; r < targets.size(); ++r)
            m_abstract_targets[ygg::uint_t(Row(targets.row(r), m_target_columns).get<Object>(database.pattern))] = true;
        if (!database.needs_update(m_abstract_targets))
            return;
        auto h = std::move(database.h);  // Reuses the table's storage.
        database.h.clear();
        h.assign(m_num_objects, ygg::database::infinity);
        m_queue.clear();
        for (ygg::uint_t o = 0; o < m_num_objects; ++o)
            if (m_abstract_targets[o])
            {
                h[o] = 0;
                m_queue.push_back(o);
            }
        for (size_t head = 0; head < m_queue.size(); ++head)
        {
            const auto o = m_queue[head];
            const auto vertex = ygg::database::encode_row<QueryValues, Object>(std::array { Object(o) }, m_vertex);
            database.edges.evaluate(database.predecessors,
                                    vertex,
                                    [&](std::span<const std::byte> edge)
                                    {
                                        const auto p = ygg::uint_t(Row(edge, database.edges.columns()).get<Object>(database.u_position));
                                        if (h[p] == ygg::database::infinity)
                                        {
                                            h[p] = h[o] + 1;
                                            m_queue.push_back(p);
                                        }
                                        return true;
                                    });
        }
        database.h = std::move(h);
        std::swap(database.targets, m_abstract_targets);
    }
};

/// The abstract edges of the pattern databases as queries, built once per feature from the translated edge query Q.
/// For each vertex column i, the pattern, the projection π_(u[i], v[i])(Q) is relaxed with only the static slots
/// known (ygg::database::relax). Relaxing the projection rather than Q eliminates the other variables early, as
/// projections of pattern databases do, so no edge over all columns is ever built. Nothing is evaluated here, and the
/// abstract edges are never materialized (PatternDatabase).
struct EdgeAbstraction
{
    ygg::database::QueryPlan<QueryValues> plan;  ///< Plan root r is the abstract edge query of column patterns[r].
    std::vector<size_t> patterns;
    std::vector<size_t> slots;                   ///< The edge query's slot of each input of the plan.
};

/// The abstraction of a translated edge query, or nullopt if no column is constrained at both ends (blind search).
template<FamilyTag Family>
std::optional<EdgeAbstraction>
abstract_edges(dl::detail::QueryTranslator<Family>& translator,
               ygg::database::QueryView<QueryValues> edges,
               const ygg::database::DistancePlan<QueryValues>& plan)
{
    const auto columns = plan.edge_columns().span();
    const auto k = plan.arity();
    auto projections = std::vector<ygg::database::QueryView<QueryValues>>();
    for (size_t i = 0; i < k; ++i)
        projections.push_back(translator.project(edges, std::array { columns[i].label, columns[k + i].label }));
    const auto& slots = translator.get_slots();
    const auto relaxation = translator.relax(projections, [&](size_t slot) { return slots[slot].is_static; });

    auto roots = std::vector<ygg::database::QueryView<QueryValues>>();
    auto patterns = std::vector<size_t>();
    for (size_t i = 0; i < k; ++i)
        if (const auto& relaxed = relaxation.queries[i]; relaxed && relaxed->columns().size() == 2)
        {
            roots.push_back(*relaxed);
            patterns.push_back(i);
        }
    if (roots.empty())
        return std::nullopt;
    return EdgeAbstraction { ygg::database::compile(roots), std::move(patterns), relaxation.slots };
}

/// The pattern database heuristic of an abstraction; `slot_rows(slot)` returns a static slot's rows. The columns of
/// u[i] and v[i] are found in each abstract edge query by label.
template<ygg::database::SlotRelations<QueryValues> SlotRows>
ProjectionHeuristic make_projection_heuristic(const EdgeAbstraction& abstraction,
                                             const ygg::database::DistancePlan<QueryValues>& plan,
                                             ygg::uint_t num_objects,
                                             const SlotRows& slot_rows)
{
    const auto columns = plan.edge_columns().span();
    const auto k = plan.arity();
    auto databases = std::vector<PatternDatabase>();
    for (size_t r = 0; r < abstraction.patterns.size(); ++r)
    {
        const auto i = abstraction.patterns[r];
        const auto query = abstraction.plan[abstraction.plan.roots()[r]];
        auto& database = databases.emplace_back(PatternDatabase { i, ygg::database::bound::QueryEvaluator<QueryValues>(query) });
        database.u_position = ygg::database::column_index(query.columns(), columns[i].label);
        database.v_position = ygg::database::column_index(query.columns(), columns[k + i].label);
        for (const auto slot : ygg::database::input_slots(std::span(&query, 1)))
            database.edges.bind_slot(slot, slot_rows(abstraction.slots[slot]));
        database.predecessors = database.edges.prepare({ database.v_position });
    }
    return ProjectionHeuristic(std::move(databases), plan.source_columns().span(), plan.target_columns().span(), num_objects);
}

}  // namespace runir::kr::dl::semantics::heuristics

#endif
