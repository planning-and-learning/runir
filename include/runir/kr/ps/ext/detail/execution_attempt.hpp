#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_ATTEMPT_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_ATTEMPT_HPP_

#include "runir/kr/ps/ext/detail/attempt_analysis.hpp"
#include "runir/kr/ps/ext/detail/search_node.hpp"

#include <cstddef>
#include <optional>
#include <tuple>
#include <vector>
#include <yggdrasil/containers/segmented_vector.hpp>

namespace runir::kr::ps::ext::detail
{

/// Selected proof transitions, reached-node rollback trail, and failure state for the current attempt.
template<tyr::TaskKind Kind>
class ExecutionAttempt
{
public:
    struct Checkpoint
    {
        std::size_t reached_size;
        std::size_t selected_edges_size;
        bool failed;
    };

private:
    std::vector<ygg::Index<ProgramState<Kind>>> m_reached;
    std::vector<std::tuple<ygg::Index<ProgramState<Kind>>, ygg::Index<ProgramState<Kind>>, ygg::uint_t>> m_selected_edges;
    bool m_failed = false;
    AttemptAnalysis<Kind> m_analysis;

public:
    void record_reached(ygg::Index<ProgramState<Kind>> state) { m_reached.push_back(state); }

    void record_transition(ygg::Index<ProgramState<Kind>> source, ygg::Index<ProgramState<Kind>> target, ygg::uint_t weight)
    {
        m_selected_edges.emplace_back(source, target, weight);
    }

    void mark_failed() { m_failed = true; }
    bool has_failed() const { return m_failed; }

    std::optional<ygg::uint_t> assess(std::size_t num_states, ProgramSearchStatistics& statistics)
    {
        return m_analysis.assess(num_states, m_selected_edges, statistics);
    }

    Checkpoint checkpoint() const { return { m_reached.size(), m_selected_edges.size(), m_failed }; }

    /// Undo only the attempt's parent links, selected transitions, and failure state; retain cumulative node data.
    void restore(const Checkpoint& checkpoint, ygg::SegmentedVector<SearchNode<Kind>>& nodes)
    {
        while (m_reached.size() > checkpoint.reached_size)
        {
            auto& node = get_or_create_search_node(m_reached.back(), nodes);
            node.parent_state = ygg::Index<ProgramState<Kind>>::max();
            node.planning_successor.reset();
            m_reached.pop_back();
        }
        m_selected_edges.resize(checkpoint.selected_edges_size);
        m_failed = checkpoint.failed;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
