#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_PROPAGATION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_PROPAGATION_HPP_

#include "runir/kr/ps/ext/detail/choice_proofs.hpp"
#include "runir/kr/ps/ext/detail/predecessors.hpp"

#include <cassert>
#include <cstddef>
#include <optional>
#include <vector>

namespace runir::kr::ps::ext::detail
{

/// Incremental least-fixed-point proof of the explored AND/OR graph.
/// Each ordinary edge is an AND requirement; each Choose rule contributes one OR requirement.
/// A goal seeds success, a successful binding satisfies its Choose once, and a state succeeds
/// when all its requirements succeed. Pure cycles cannot justify themselves.
///
/// This is counter-based Horn propagation: children_1 AND ... AND children_n -> state,
/// and binding -> Choose. See Dowling and Gallier (1984), "Linear-time algorithms for
/// testing the satisfiability of propositional Horn formulae":
/// https://doi.org/10.1016/0743-1066(84)90014-1
/// See also the reachability-game formulation, Algorithm 1:
/// https://lsv.ens-paris-saclay.fr/~dwb/gtc.pdf
///
/// Only sealed nonterminal states can succeed: enumeration must finish before their AND requirements
/// are complete. Choose alternatives may remain lazy because one proved binding suffices.
/// Every state becomes successful at most once and each recorded dependency is credited
/// at most once. Resolution takes O(V + E + C) time and space for indexed state slots,
/// recorded edges and Choose obligations, excluding successor generation.
template<tyr::TaskKind Kind>
class ProofPropagation
{
private:
    struct State
    {
        std::size_t remaining = 0;
        std::optional<EdgeId> first_incoming;
        bool sealed = false;
    };

    // Indexed exactly like Predecessors; source/target are not duplicated.
    struct Dependency
    {
        std::optional<EdgeId> next;
        std::optional<ChoiceId> choice_id;
    };

    ygg::SegmentedVector<SearchNode<Kind>>& m_nodes;
    const Predecessors<Kind>& m_predecessors;
    std::vector<State> m_states;
    std::vector<Dependency> m_dependencies;
    ChoiceProofs m_choice_proofs;
    std::vector<ProgramStateView<Kind>> m_ready;

public:
    ProofPropagation(ygg::SegmentedVector<SearchNode<Kind>>& nodes, const Predecessors<Kind>& predecessors) : m_nodes(nodes), m_predecessors(predecessors) {}

    /// Each enabled Choose is a distinct requirement, including an empty one that can never succeed.
    ChoiceId add_choice(ProgramStateView<Kind> state)
    {
        auto& proof = state_for(state);
        assert(!proof.sealed);
        ++proof.remaining;
        return m_choice_proofs.create();
    }

    bool choice_succeeded(ChoiceId choice_id) const { return m_choice_proofs.is_satisfied(choice_id); }

    /// Register one diagnostic edge as an ordinary requirement or a binding of an existing Choose.
    void add_transition(EdgeId edge, std::optional<ChoiceId> choice_id)
    {
        assert(static_cast<ygg::uint_t>(edge) == m_dependencies.size());
        const auto& transition = m_predecessors[edge];
        if (!choice_id)
        {
            auto& source = state_for(transition.source);
            assert(!source.sealed);
            ++source.remaining;
        }
        m_dependencies.push_back({ std::nullopt, choice_id });
        if (m_nodes[ygg::uint_t(transition.target.get_index())].status == SearchStatus::SUCCESS)
        {
            // Its success event may still be queued. Credit now WITHOUT linking it again.
            satisfy(edge);
        }
        else
        {
            auto& target = state_for(transition.target);
            m_dependencies.back().next = target.first_incoming;
            target.first_incoming = edge;
        }
    }

    /// Enumeration has finished; no further ordinary edges or Choose requirements will be added.
    void seal(ProgramStateView<Kind> state)
    {
        state_for(state).sealed = true;
        try_prove(state);
    }

    /// Called for terminal goals and for states whose complete requirements have been discharged.
    void succeed(ProgramStateView<Kind> state)
    {
        auto& node = m_nodes[ygg::uint_t(state.get_index())];
        if (node.status != SearchStatus::SUCCESS)
        {
            node.status = SearchStatus::SUCCESS;
            m_ready.push_back(state);
        }
    }

    /// Drain only newly successful states. On interruption return false; the caller ends the search.
    template<typename Stop>
    bool propagate(Stop&& stop)
    {
        for (std::size_t i = 0; i < m_ready.size(); ++i)
        {
            if (stop())
                return false;
            for (auto edge = state_for(m_ready[i]).first_incoming; edge; edge = m_dependencies[static_cast<ygg::uint_t>(*edge)].next)
            {
                if (stop())
                    return false;
                satisfy(*edge);
            }
        }
        m_ready.clear();
        return true;
    }

private:
    State& state_for(ProgramStateView<Kind> state)
    {
        if (m_states.size() < m_nodes.size())
            m_states.resize(m_nodes.size());
        return m_states[ygg::uint_t(state.get_index())];
    }

    void satisfy(EdgeId edge)
    {
        if (const auto choice_id = m_dependencies[static_cast<ygg::uint_t>(edge)].choice_id; choice_id && !m_choice_proofs.satisfy(*choice_id))
            return;
        const auto source = m_predecessors[edge].source;
        auto& state = state_for(source);
        assert(state.remaining != 0);
        --state.remaining;
        try_prove(source);
    }

    void try_prove(ProgramStateView<Kind> state)
    {
        const auto& proof = state_for(state);
        const auto& node = m_nodes[ygg::uint_t(state.get_index())];
        if (proof.sealed && proof.remaining == 0 && !node.is_open && !node.is_deadend && !node.is_unsolvable)
            succeed(state);
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
