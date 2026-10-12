#ifndef RUNIR_KR_DL_SEMANTICS_HEURISTICS_PROJECTION_HPP_
#define RUNIR_KR_DL_SEMANTICS_HEURISTICS_PROJECTION_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <tyr/formalism/declarations.hpp>
#include <vector>
#include <yggdrasil/database/optimization/optimization.hpp>
#include <yggdrasil/database/semantics/distance.hpp>
#include <yggdrasil/database/semantics/operations.hpp>
#include <yggdrasil/database/semantics/query_evaluation.hpp>
#include <yggdrasil/database/syntax/query.hpp>

namespace runir::kr::dl::semantics::heuristics
{

using Object = ygg::Index<tyr::formalism::Object>;
using Row = ygg::database::Row<QueryValues>;

/// The pattern database of one vertex column i, the pattern: the abstraction α_i(v) = v[i] maps the edges to a graph
/// over objects, and h is its breadth-first distance table to the abstract targets α_i(T).
struct PatternDatabase
{
    size_t pattern = 0;                             ///< The vertex column i.
    std::vector<ygg::uint_t> offsets;               ///< CSR: the abstract predecessors of o are at [offsets[o], offsets[o + 1]).
    std::vector<ygg::uint_t> abstract_predecessors;
    std::vector<bool> targets;                      ///< α_i(T), for which h was computed.
    std::vector<ygg::uint_t> h;                     ///< Abstract distance to α_i(T); uint_t's maximum if unreachable.
    bool informative = false;                       ///< Some entry of h is nonzero.
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
    static constexpr ygg::uint_t infinity = std::numeric_limits<ygg::uint_t>::max();

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

    /// Recomputes the tables whose abstract targets changed.
    template<typename Targets>
    void update(const Targets& targets)
    {
        m_informative.clear();
        for (size_t i = 0; i < m_databases.size(); ++i)
        {
            update_table(m_databases[i], targets);
            if (m_databases[i].informative)
                m_informative.push_back(i);
        }
    }

    ygg::uint_t estimate(std::span<const std::byte> vertex) const
    {
        ygg::uint_t value = 0;
        for (const auto i : m_informative)
        {
            const auto& database = m_databases[i];
            const auto entry = database.h[ygg::uint_t(Row(vertex, m_vertex_columns).get<Object>(database.pattern))];
            if (entry == infinity)
                return entry;
            value = std::max(value, entry);
        }
        return value;
    }

private:
    std::vector<PatternDatabase> m_databases;
    std::vector<size_t> m_informative;  ///< The databases with some nonzero entry.
    std::vector<ygg::database::ColumnLayout> m_vertex_columns;
    std::vector<ygg::database::ColumnLayout> m_target_columns;
    ygg::uint_t m_num_objects = 0;

    /// A backward breadth-first search from α_i(T) in the abstract graph.
    template<typename Targets>
    void update_table(PatternDatabase& database, const Targets& targets) const
    {
        // ponytail: comparing the abstract targets costs O(|targets|) per evaluation.
        auto abstract_targets = std::vector<bool>(m_num_objects, false);
        for (size_t r = 0; r < targets.size(); ++r)
            abstract_targets[ygg::uint_t(Row(targets.row(r), m_target_columns).get<Object>(database.pattern))] = true;
        if (!database.h.empty() && abstract_targets == database.targets)
            return;
        database.targets = std::move(abstract_targets);
        database.h.assign(m_num_objects, infinity);
        auto queue = std::vector<ygg::uint_t>();
        for (ygg::uint_t o = 0; o < m_num_objects; ++o)
            if (database.targets[o])
            {
                database.h[o] = 0;
                queue.push_back(o);
            }
        for (size_t head = 0; head < queue.size(); ++head)
        {
            const auto o = queue[head];
            for (auto i = database.offsets[o]; i < database.offsets[o + 1]; ++i)
                if (const auto p = database.abstract_predecessors[i]; database.h[p] == infinity)
                {
                    database.h[p] = database.h[o] + 1;
                    queue.push_back(p);
                }
        }
        database.informative = std::ranges::any_of(database.h, [](ygg::uint_t value) { return value != 0; });
    }
};

/// The pattern database of vertex column `pattern` from its abstract edges, the rows (u[i], v[i]): a CSR of the
/// abstract predecessors of each object; the distance table is computed by ProjectionHeuristic::update().
template<typename Relation>
PatternDatabase pattern_database(size_t pattern, const Relation& abstract_edges, ygg::uint_t num_objects)
{
    auto database = PatternDatabase {};
    database.pattern = pattern;
    database.offsets.assign(num_objects + 1, 0);
    const auto layout = abstract_edges.columns().span();
    for (size_t e = 0; e < abstract_edges.size(); ++e)
        ++database.offsets[ygg::uint_t(Row(abstract_edges.row(e), layout).get<Object>(1)) + 1];
    for (size_t o = 0; o < num_objects; ++o)
        database.offsets[o + 1] += database.offsets[o];
    database.abstract_predecessors.resize(database.offsets.back());
    auto fill = std::vector<ygg::uint_t>(database.offsets.begin(), database.offsets.end() - 1);
    for (size_t e = 0; e < abstract_edges.size(); ++e)
    {
        const auto row = Row(abstract_edges.row(e), layout);
        database.abstract_predecessors[fill[ygg::uint_t(row.get<Object>(1))]++] = ygg::uint_t(row.get<Object>(0));
    }
    return database;
}

/// The abstract edges of the pattern databases as queries, built once per feature from the translated edge query Q.
/// For each vertex column i, the pattern, the projection π_(u[i], v[i])(Q) is relaxed with only the static slots
/// known (ygg::database::relax). Relaxing the projection rather than Q eliminates the other variables early, as
/// projections of pattern databases do, so no edge over all columns is ever built. Nothing is evaluated here.
struct EdgeAbstraction
{
    ygg::database::QueryPlan<QueryValues> plan;  ///< Plan root r is the abstract edge query of column patterns[r].
    std::vector<size_t> patterns;
    std::vector<size_t> slots;                   ///< The edge query's slot of each input of the plan.
};

/// The abstraction of a translated edge query, or nullopt if no column is constrained at both ends (blind search).
template<typename Translator>
std::optional<EdgeAbstraction>
abstract_edges(Translator& translator, ygg::database::QueryView<QueryValues> edges, const ygg::database::DistancePlan<QueryValues>& plan)
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

/// The pattern database heuristic of an abstraction; `slot_rows(slot)` returns a static slot's rows.
template<typename SlotRows>
ProjectionHeuristic make_projection_heuristic(const EdgeAbstraction& abstraction,
                                             const ygg::database::DistancePlan<QueryValues>& plan,
                                             ygg::uint_t num_objects,
                                             SlotRows&& slot_rows)
{
    // The abstract edges, evaluated once over the static rows.
    auto inputs = std::vector<ygg::Builder<ygg::database::Relation<QueryValues>>>();
    for (const auto slot : abstraction.slots)
    {
        const auto& rows = slot_rows(slot);
        ygg::database::assign(inputs.emplace_back(rows.columns().span()), rows);
    }
    auto roots = std::vector<ygg::database::QueryView<QueryValues>>();
    for (const auto root : abstraction.plan.roots())
        roots.push_back(abstraction.plan[root]);
    auto evaluator = ygg::database::QueryEvaluator<QueryValues>(ygg::database::optimize<QueryValues>(roots, ygg::database::collect_statistics<QueryValues>(inputs)));
    evaluator.evaluate(inputs);

    auto databases = std::vector<PatternDatabase>();
    for (size_t r = 0; r < abstraction.patterns.size(); ++r)
        databases.push_back(pattern_database(abstraction.patterns[r], evaluator.get_result(r), num_objects));
    return ProjectionHeuristic(std::move(databases), plan.source_columns().span(), plan.target_columns().span(), num_objects);
}

}  // namespace runir::kr::dl::semantics::heuristics

#endif
