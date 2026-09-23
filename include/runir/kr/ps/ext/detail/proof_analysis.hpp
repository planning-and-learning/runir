#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_ANALYSIS_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_ANALYSIS_HPP_

#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/search_node.hpp"

#include <cstddef>
#include <limits>
#include <span>
#include <vector>

namespace runir::kr::ps::ext::detail
{

/// Postprocess the explored transitions after the ordinary frontier has drained.
/// Scratch storage is reused; no proof status is stored in search nodes.
template<tyr::TaskKind Kind>
class ProofAnalysis
{
private:
    std::vector<std::size_t> m_pending;
    std::vector<std::size_t> m_heads;
    std::vector<std::size_t> m_next_edge;
    std::vector<std::size_t> m_queue;
    std::vector<bool> m_proved;
    std::vector<bool> m_proved_choices;

public:
    bool choice_is_proved(std::size_t index) const { return m_proved_choices[index]; }

    template<typename Stop>
    bool assess(const ygg::SegmentedVector<SearchNode<Kind>>& nodes,
                std::span<const Predecessor<Kind>> edges,
                std::span<const ChoiceFrame<Kind>> choices,
                ygg::Index<ProgramState<Kind>> initial,
                Stop&& stop)
    {
        constexpr auto no_edge = std::numeric_limits<std::size_t>::max();
        m_pending.assign(nodes.size(), 0);
        m_heads.assign(nodes.size(), no_edge);
        m_next_edge.resize(edges.size());
        m_proved.assign(nodes.size(), false);
        m_proved_choices.assign(choices.size(), false);
        m_queue.clear();
        for (const auto& choice : choices)
        {
            if (stop())
                return false;
            ++m_pending[ygg::uint_t(choice.state)];
        }
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            if (stop())
                return false;
            const auto& edge = edges[i];
            if (!edge.choice)
                ++m_pending[ygg::uint_t(edge.source)];
            const auto target = ygg::uint_t(edge.target);
            m_next_edge[i] = m_heads[target];
            m_heads[target] = i;
        }
        for (std::size_t i = 0; i < nodes.size(); ++i)
        {
            if (stop())
                return false;
            if (nodes[i].is_goal)
            {
                m_proved[i] = true;
                m_queue.push_back(i);
            }
        }

        // Goals justify finite proofs. Cycles cannot prove themselves, while a later
        // Choose alternative can justify a previously explored cyclic dependency.
        for (std::size_t i = 0; i < m_queue.size(); ++i)
        {
            const auto target = m_queue[i];
            for (auto index = m_heads[target]; index != no_edge; index = m_next_edge[index])
            {
                if (stop())
                    return false;
                const auto& edge = edges[index];
                if (edge.choice)
                {
                    if (m_proved_choices[*edge.choice])
                        continue;
                    m_proved_choices[*edge.choice] = true;
                }
                const auto source = ygg::uint_t(edge.source);
                if (--m_pending[source] == 0 && !nodes[source].is_open && !nodes[source].is_unsolvable)
                {
                    m_proved[source] = true;
                    m_queue.push_back(source);
                }
            }
        }
        return m_proved[ygg::uint_t(initial)];
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
