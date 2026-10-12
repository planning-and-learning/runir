#ifndef RUNIR_KR_DL_SEMANTICS_HEURISTICS_PROJECTION_HPP_
#define RUNIR_KR_DL_SEMANTICS_HEURISTICS_PROJECTION_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/semantics/heuristics/query_relaxation.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <span>
#include <tyr/formalism/declarations.hpp>
#include <vector>
#include <yggdrasil/database/semantics/operations.hpp>

namespace runir::kr::dl::semantics::heuristics
{

using Object = ygg::Index<tyr::formalism::Object>;

/// The object in a vertex column of a row.
inline Object object(std::span<const std::byte> row, std::span<const ygg::database::ColumnLayout> layout, size_t position)
{
    return ygg::database::detail::read_column<QueryValues, Object>(row, layout, position);
}

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
/// Speeds up Heuristic Search", AIJ 2006). The abstract edges come from a static relaxation of the edge query projected
/// onto (u[i], v[i]) (query_relaxation.hpp), a superset of the projected edges in every state. Every edge u -> v thus
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
            const auto entry = database.h[ygg::uint_t(object(vertex, m_vertex_columns, database.pattern))];
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
            abstract_targets[ygg::uint_t(object(targets.row(r), m_target_columns, database.pattern))] = true;
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

/// The pattern databases of an edge query: one per vertex column whose relaxed projection constrains both endpoints.
template<FamilyTag Family, typename C, typename Leaves>
std::vector<PatternDatabase> pattern_databases(ygg::View<ygg::Index<Query<Family>>, C> edges,
                                               std::span<const ygg::database::ColumnLayout> edge_columns,
                                               size_t k,
                                               ygg::uint_t num_objects,
                                               detail::QueryRelaxation<Family, Leaves>& relaxation)
{
    using Relaxed = typename detail::QueryRelaxation<Family, Leaves>::Relaxed;
    auto databases = std::vector<PatternDatabase>();
    for (size_t i = 0; i < k; ++i)
    {
        const auto from = edge_columns[i].label;
        const auto to = edge_columns[k + i].label;
        const auto pair = std::array<ygg::Index<ygg::database::Column>, 2> { from, to };
        const auto relaxed = relaxation.relax(edges, pair);
        // Free or a single endpoint: no information. Empty: no edges in any state, so only targets are finite.
        if (relaxed.kind == Relaxed::Kind::FREE || (relaxed.kind == Relaxed::Kind::ROWS && relaxed.rows->arity() != 2))
            continue;
        auto& database = databases.emplace_back();
        database.pattern = i;
        database.offsets.assign(num_objects + 1, 0);
        if (relaxed.kind == Relaxed::Kind::EMPTY)
            continue;
        const auto& rows = *relaxed.rows;
        const auto layout = rows.columns().span();
        const auto from_position = size_t(rows.column_index(from));
        const auto to_position = size_t(rows.column_index(to));
        for (size_t r = 0; r < rows.size(); ++r)
            ++database.offsets[ygg::uint_t(object(rows.row(r), layout, to_position)) + 1];
        for (size_t o = 0; o < num_objects; ++o)
            database.offsets[o + 1] += database.offsets[o];
        database.abstract_predecessors.resize(database.offsets.back());
        auto fill = std::vector<ygg::uint_t>(database.offsets.begin(), database.offsets.end() - 1);
        for (size_t r = 0; r < rows.size(); ++r)
        {
            const auto row = rows.row(r);
            database.abstract_predecessors[fill[ygg::uint_t(object(row, layout, to_position))]++] = ygg::uint_t(object(row, layout, from_position));
        }
    }
    return databases;
}

/// The pattern database heuristic of an edge query; `leaves(index)` returns a static leaf's rows.
template<FamilyTag Family, typename C, typename Leaves>
ProjectionHeuristic make_projection_heuristic(ygg::View<ygg::Index<Query<Family>>, C> edges,
                                             const ygg::database::DistancePlan<QueryValues>& plan,
                                             ygg::uint_t num_objects,
                                             Leaves leaves)
{
    auto relaxation = detail::QueryRelaxation<Family, Leaves>(std::move(leaves));
    auto databases = pattern_databases<Family>(edges, plan.edge_columns().span(), plan.arity(), num_objects, relaxation);
    return ProjectionHeuristic(std::move(databases), plan.source_columns().span(), plan.target_columns().span(), num_objects);
}

}  // namespace runir::kr::dl::semantics::heuristics

#endif
