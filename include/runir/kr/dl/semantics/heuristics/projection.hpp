#ifndef RUNIR_KR_DL_SEMANTICS_HEURISTICS_PROJECTION_HPP_
#define RUNIR_KR_DL_SEMANTICS_HEURISTICS_PROJECTION_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <tyr/formalism/declarations.hpp>
#include <vector>
#include <yggdrasil/database/semantics/operations.hpp>

namespace runir::kr::dl::semantics::heuristics
{

/// The abstraction of one vertex column: a graph over objects (as predecessor lists) and its distance table to the
/// projected targets.
struct ProjectionColumn
{
    size_t position = 0;               ///< The vertex column this projection abstracts.
    std::vector<ygg::uint_t> offsets;  ///< CSR: the predecessors of object o are preds[offsets[o]..offsets[o + 1]).
    std::vector<ygg::uint_t> preds;
    std::vector<bool> targets;         ///< The projected targets the table was computed for.
    std::vector<ygg::uint_t> table;    ///< Abstract distance to the targets; uint_t's maximum if unreachable.
    bool informative = false;          ///< Some table entry is nonzero.
};

/// The maximum over vertex columns of a distance table in an abstraction of the edge relation, as pattern databases
/// combine projections (Culberson & Schaeffer, "Pattern Databases", Computational Intelligence 1998; maximizing over
/// several, Holte et al., "Maximizing over Multiple Pattern Databases Speeds up Heuristic Search", AIJ 2006). Each
/// column's abstraction is a static relaxation of the edge query projected onto that column (query_relaxation.hpp),
/// a superset of the edges in every state, so the estimate is consistent in every state; only the tables depend on the
/// targets.
class ProjectionHeuristic
{
public:
    static constexpr ygg::uint_t infinity = std::numeric_limits<ygg::uint_t>::max();

    /// The zero estimate until columns are given.
    ProjectionHeuristic() = default;
    ProjectionHeuristic(std::vector<ProjectionColumn> columns,
                        std::span<const ygg::database::ColumnLayout> vertex_columns,
                        std::span<const ygg::database::ColumnLayout> target_columns,
                        ygg::uint_t num_objects) :
        m_columns(std::move(columns)),
        m_vertex_columns(vertex_columns.begin(), vertex_columns.end()),
        m_target_columns(target_columns.begin(), target_columns.end()),
        m_num_objects(num_objects)
    {
    }

    /// Recomputes the tables whose projected targets changed.
    template<typename Targets>
    void update(const Targets& targets)
    {
        m_informative.clear();
        for (size_t i = 0; i < m_columns.size(); ++i)
        {
            update_table(m_columns[i], targets);
            if (m_columns[i].informative)
                m_informative.push_back(i);
        }
    }

    ygg::uint_t estimate(std::span<const std::byte> vertex) const
    {
        ygg::uint_t value = 0;
        for (const auto i : m_informative)
        {
            const auto& column = m_columns[i];
            const auto entry = column.table[ygg::uint_t(object(vertex, m_vertex_columns, column.position))];
            if (entry == infinity)
                return entry;
            value = std::max(value, entry);
        }
        return value;
    }

private:
    std::vector<ProjectionColumn> m_columns;
    std::vector<size_t> m_informative;  ///< The columns with some nonzero entry.
    std::vector<ygg::database::ColumnLayout> m_vertex_columns;
    std::vector<ygg::database::ColumnLayout> m_target_columns;
    ygg::uint_t m_num_objects = 0;

    static ygg::Index<tyr::formalism::Object> object(std::span<const std::byte> row, std::span<const ygg::database::ColumnLayout> layout, size_t position)
    {
        return ygg::database::detail::read_column<QueryValues, ygg::Index<tyr::formalism::Object>>(row, layout, position);
    }

    /// A backward breadth-first search from the projected targets in the column's abstract graph.
    template<typename Targets>
    void update_table(ProjectionColumn& column, const Targets& targets) const
    {
        // ponytail: comparing the projected targets costs O(|targets|) per evaluation.
        auto projected = std::vector<bool>(m_num_objects, false);
        for (size_t r = 0; r < targets.size(); ++r)
            projected[ygg::uint_t(object(targets.row(r), m_target_columns, column.position))] = true;
        if (!column.table.empty() && projected == column.targets)
            return;
        column.targets = std::move(projected);
        column.table.assign(m_num_objects, infinity);
        auto queue = std::vector<ygg::uint_t>();
        for (ygg::uint_t o = 0; o < m_num_objects; ++o)
            if (column.targets[o])
            {
                column.table[o] = 0;
                queue.push_back(o);
            }
        for (size_t head = 0; head < queue.size(); ++head)
        {
            const auto o = queue[head];
            for (auto i = column.offsets[o]; i < column.offsets[o + 1]; ++i)
                if (const auto p = column.preds[i]; column.table[p] == infinity)
                {
                    column.table[p] = column.table[o] + 1;
                    queue.push_back(p);
                }
        }
        column.informative = std::ranges::any_of(column.table, [](ygg::uint_t value) { return value != 0; });
    }
};

}  // namespace runir::kr::dl::semantics::heuristics

#endif
