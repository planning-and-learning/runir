#ifndef RUNIR_KR_PS_EXT_DETAIL_ATTEMPT_ANALYSIS_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_ATTEMPT_ANALYSIS_HPP_

#include "runir/kr/ps/ext/execution_index.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <tuple>
#include <vector>

namespace runir::kr::ps::ext::detail
{

/// Analyzes the selected transitions of one attempt; scratch storage survives retries.
template<tyr::TaskKind Kind>
class AttemptAnalysis
{
private:
    std::vector<std::size_t> m_indegrees;
    std::vector<std::size_t> m_heads;
    std::vector<std::size_t> m_next_edge;
    std::vector<ygg::uint_t> m_depths;
    std::vector<std::size_t> m_queue;

public:
    /// Returns the longest choice depth, or nullopt if the attempt contains a cycle.
    std::optional<ygg::uint_t> assess(std::size_t num_states,
                                      std::span<const std::tuple<ygg::Index<ProgramState<Kind>>, ygg::Index<ProgramState<Kind>>, ygg::uint_t>> edges,
                                      ProgramSearchStatistics& statistics)
    {
        constexpr auto no_edge = std::numeric_limits<std::size_t>::max();
        m_indegrees.assign(num_states, 0);
        m_heads.assign(num_states, no_edge);
        m_depths.assign(num_states, 0);
        m_next_edge.resize(edges.size());
        m_queue.clear();
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto [source, target, weight] = edges[i];
            ++m_indegrees[ygg::uint_t(target)];
            m_next_edge[i] = m_heads[ygg::uint_t(source)];
            m_heads[ygg::uint_t(source)] = i;
        }
        for (std::size_t i = 0; i < num_states; ++i)
            if (m_indegrees[i] == 0)
                m_queue.push_back(i);

        // Shared continuations retain the longest incoming path, regardless of expansion order.
        ygg::uint_t depth = 0;
        for (std::size_t i = 0; i < m_queue.size(); ++i)
        {
            const auto source = m_queue[i];
            depth = std::max(depth, m_depths[source]);
            for (auto edge = m_heads[source]; edge != no_edge; edge = m_next_edge[edge])
            {
                const auto [_, target, weight] = edges[edge];
                const auto index = ygg::uint_t(target);
                m_depths[index] = std::max(m_depths[index], m_depths[source] + weight);
                if (--m_indegrees[index] == 0)
                    m_queue.push_back(index);
            }
        }
        // Cyclic attempts retain their observed traversal peak instead of following cycles repeatedly.
        if (m_queue.size() != num_states)
            return std::nullopt;
        statistics.max_choice_depth = std::max(statistics.max_choice_depth, depth);
        return depth;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
