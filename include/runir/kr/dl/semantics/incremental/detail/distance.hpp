#ifndef RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_DISTANCE_HPP_
#define RUNIR_KR_DL_SEMANTICS_INCREMENTAL_DETAIL_DISTANCE_HPP_

#include "runir/kr/dl/semantics/incremental/denotation_delta.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace runir::kr::dl::semantics::incremental::detail
{

/// Retains shortest distances from every source, reverse edges, and the number
/// of predecessors supporting each shortest distance. Deletions invalidate only
/// vertices losing all such support; a min-heap repairs their distances from the
/// surviving boundary and propagates improvements from inserted edges/sources.
/// Work follows the affected vertices and their incident bitset rows, rather
/// than a fresh BFS. Target changes select among the retained distances.
/// Buffers retain capacity; the pending heap grows to its observed high-water mark.
class DistanceEvaluator
{
    static constexpr auto infinity = std::numeric_limits<ygg::uint_t>::max();

    ygg::Builder<Denotation<RoleTag>> m_reverse;
    std::vector<ygg::uint_t> m_distances;
    std::vector<size_t> m_supports;
    std::vector<ygg::uint_t> m_invalidated;
    std::vector<std::pair<ygg::uint_t, ygg::uint_t>> m_pending;
    std::vector<ygg::uint_t> m_dirty;
    std::vector<unsigned char> m_dirty_flags;
    ygg::uint_t m_result = infinity;
    bool m_initialized = false;

    void remove_support(ygg::uint_t object);
    void relax(ygg::uint_t object, ygg::uint_t distance);
    void mark_dirty(ygg::uint_t object);
    void select_targets(BorrowedDenotationView<ConceptTag> targets);

public:
    void initialize(BorrowedDenotationView<ConceptTag> sources, BorrowedDenotationView<RoleTag> edges, BorrowedDenotationView<ConceptTag> targets);

    /// Views contain the final state; deltas are disjoint, actual changes from
    /// the preceding state, over the same object universe. Reinitialize after
    /// an unsuccessful update or when crossing an invocation boundary.
    void update(BorrowedDenotationView<ConceptTag> sources,
                BorrowedDenotationView<RoleTag> edges,
                BorrowedDenotationView<ConceptTag> targets,
                const DenotationDelta<ConceptTag>& source_delta,
                const DenotationDelta<RoleTag>& edge_delta,
                const DenotationDelta<ConceptTag>& target_delta);

    ygg::uint_t get_result() const;
};

inline void DistanceEvaluator::remove_support(ygg::uint_t object)
{
    assert(m_supports[object] != 0);
    if (--m_supports[object] == 0)
        m_invalidated.push_back(object);
}

inline void DistanceEvaluator::relax(ygg::uint_t object, ygg::uint_t distance)
{
    if (distance >= m_distances[object])
        return;
    m_distances[object] = distance;
    m_pending.emplace_back(distance, object);
    std::push_heap(m_pending.begin(), m_pending.end(), std::greater {});
}

inline void DistanceEvaluator::mark_dirty(ygg::uint_t object)
{
    if (!m_dirty_flags[object])
    {
        m_dirty_flags[object] = true;
        m_dirty.push_back(object);
    }
}

inline void DistanceEvaluator::select_targets(BorrowedDenotationView<ConceptTag> targets)
{
    m_result = infinity;
    for (const auto object : ygg::set_bit_indices(targets.get()))
        m_result = std::min(m_result, m_distances[object]);
}

inline void
DistanceEvaluator::initialize(BorrowedDenotationView<ConceptTag> sources, BorrowedDenotationView<RoleTag> edges, BorrowedDenotationView<ConceptTag> targets)
{
    m_initialized = false;
    const auto size = sources.get_num_objects();
    if (edges.get_num_objects() != size || targets.get_num_objects() != size)
        throw std::invalid_argument("Incremental distance: different object universes.");
    m_reverse.initialize(size);
    m_distances.assign(size, infinity);
    m_supports.assign(size, 0);
    m_dirty_flags.assign(size, false);
    m_invalidated.clear();
    m_invalidated.reserve(size);
    m_pending.clear();
    m_pending.reserve(size);
    m_dirty.clear();
    m_dirty.reserve(size);
    for (ygg::uint_t source = 0; source < size; ++source)
        for (const auto target : ygg::set_bit_indices(edges.get(source)))
            m_reverse.get(static_cast<ygg::uint_t>(target)).set(source);

    for (const auto source : ygg::set_bit_indices(sources.get()))
    {
        m_distances[source] = 0;
        m_supports[source] = 1;  // A source has its own zero-length path.
        m_invalidated.push_back(static_cast<ygg::uint_t>(source));
    }
    for (size_t position = 0; position < m_invalidated.size(); ++position)
    {
        const auto source = m_invalidated[position];
        const auto distance = m_distances[source] + 1;
        for (const auto target : ygg::set_bit_indices(edges.get(source)))
        {
            if (m_distances[target] == infinity)
            {
                m_distances[target] = distance;
                m_invalidated.push_back(static_cast<ygg::uint_t>(target));
            }
            if (m_distances[target] == distance)
                ++m_supports[target];
        }
    }
    m_invalidated.clear();
    select_targets(targets);
    m_initialized = true;
}

inline void DistanceEvaluator::update(BorrowedDenotationView<ConceptTag> sources,
                                      BorrowedDenotationView<RoleTag> edges,
                                      BorrowedDenotationView<ConceptTag> targets,
                                      const DenotationDelta<ConceptTag>& source_delta,
                                      const DenotationDelta<RoleTag>& edge_delta,
                                      const DenotationDelta<ConceptTag>& target_delta)
{
    if (!m_initialized)
        throw std::logic_error("Incremental distance must be initialized before updating.");
    m_initialized = false;
    const auto size = m_distances.size();
    if (sources.get_num_objects() != size || edges.get_num_objects() != size || targets.get_num_objects() != size)
        throw std::invalid_argument("Incremental distance: different object universes.");

    // No old source remains to anchor incremental repair. Restart the unit-edge
    // BFS instead of invalidating and repairing every reachable vertex.
    if (!source_delta.removed.empty() && sources.get().count() == source_delta.added.size())
    {
        initialize(sources, edges, targets);
        return;
    }

    // Remove all deleted shortest-path supports before changing any distance.
    // The reverse relation temporarily holds exactly the surviving old edges.
    for (const auto& [source, target] : edge_delta.removed)
    {
        const auto from = ygg::uint_t(source.get_index());
        const auto to = ygg::uint_t(target.get_index());
        if (!m_reverse.get(to)[from])
            throw std::invalid_argument("Incremental distance: removed edge is absent.");
        m_reverse.get(to).reset(from);
        if (m_distances[from] != infinity && m_distances[to] == m_distances[from] + 1)
            remove_support(to);
    }
    for (const auto source : source_delta.removed)
    {
        const auto object = ygg::uint_t(source.get_index());
        if (m_distances[object] != 0)
            throw std::invalid_argument("Incremental distance: removed source is absent.");
        remove_support(object);
    }
    for (size_t position = 0; position < m_invalidated.size(); ++position)
    {
        const auto source = m_invalidated[position];
        const auto distance = m_distances[source];
        m_distances[source] = infinity;
        for (const auto target : ygg::set_bit_indices(edges.get(source)))
            if (m_reverse.get(static_cast<ygg::uint_t>(target))[source] && m_distances[target] == distance + 1)
                remove_support(static_cast<ygg::uint_t>(target));
    }

    // Insertions cannot invalidate surviving paths. Seed their improvements,
    // then repair invalidated vertices from finite incoming boundary distances.
    for (const auto& [source, target] : edge_delta.added)
    {
        const auto from = ygg::uint_t(source.get_index());
        const auto to = ygg::uint_t(target.get_index());
        if (m_reverse.get(to)[from])
            throw std::invalid_argument("Incremental distance: added edge is already present.");
        m_reverse.get(to).set(from);
        mark_dirty(to);
        if (m_distances[from] != infinity)
            relax(to, m_distances[from] + 1);
    }
    for (const auto source : source_delta.added)
    {
        const auto object = ygg::uint_t(source.get_index());
        if (m_distances[object] == 0)
            throw std::invalid_argument("Incremental distance: added source is already present.");
        relax(object, 0);
    }
    for (const auto target : m_invalidated)
        for (const auto source : ygg::set_bit_indices(m_reverse.get(target)))
            if (m_distances[source] != infinity)
                relax(target, m_distances[source] + 1);

    const bool distances_changed = !m_invalidated.empty() || !m_pending.empty();
    while (!m_pending.empty())
    {
        std::pop_heap(m_pending.begin(), m_pending.end(), std::greater {});
        const auto [distance, source] = m_pending.back();
        m_pending.pop_back();
        if (distance != m_distances[source])
            continue;
        mark_dirty(source);
        for (const auto target : ygg::set_bit_indices(edges.get(source)))
        {
            mark_dirty(static_cast<ygg::uint_t>(target));
            relax(static_cast<ygg::uint_t>(target), distance + 1);
        }
    }

    // Recount only vertices whose shortest predecessors may have changed.
    for (const auto target : m_dirty)
    {
        m_dirty_flags[target] = false;
        auto& support = m_supports[target];
        support = 0;
        if (sources.get()[target])
            support = 1;
        else if (m_distances[target] != infinity)
            for (const auto source : ygg::set_bit_indices(m_reverse.get(target)))
                if (m_distances[source] != infinity && m_distances[source] + 1 == m_distances[target])
                    ++support;
        assert(m_distances[target] == infinity || support != 0);
    }
    m_dirty.clear();
    m_invalidated.clear();

    if (distances_changed || std::ranges::any_of(target_delta.removed, [&](auto target) { return m_distances[ygg::uint_t(target.get_index())] == m_result; }))
        select_targets(targets);
    else
        for (const auto target : target_delta.added)
            m_result = std::min(m_result, m_distances[ygg::uint_t(target.get_index())]);
    m_initialized = true;
}

inline ygg::uint_t DistanceEvaluator::get_result() const
{
    if (!m_initialized)
        throw std::logic_error("Incremental distance has no initialized result.");
    return m_result;
}

}  // namespace runir::kr::dl::semantics::incremental::detail

#endif
