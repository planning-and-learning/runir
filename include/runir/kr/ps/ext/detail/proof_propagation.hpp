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
/// Nonterminal states can succeed only after all requirements selected by the execution mode
/// have been declared and satisfied. Choose alternatives may remain lazy because one proved binding suffices.
/// Every state becomes successful at most once and each recorded dependency is credited
/// at most once. Resolution takes O(V + E + C) time and space for indexed state slots,
/// recorded edges and Choose obligations, excluding successor generation.
template<tyr::TaskKind Kind>
class ProofPropagation
{
private:
    /// Proof requirements and reverse dependencies of one program state.
    struct StateProof
    {
        // Unsatisfied requirements: one per ordinary edge and one per Choose rule, not per binding.
        std::size_t remaining_requirements = 0;
        // Incoming dependencies to notify when this state succeeds; linked by Dependency::next_incoming_edge.
        std::optional<EdgeId> first_incoming_edge;
        // All requirements have been declared. Zero remaining during enumeration is not yet conclusive:
        // an already-successful child may satisfy an edge before further requirements are added.
        bool enumeration_complete = false;
    };

    // Indexed exactly like Predecessors; source/target are not duplicated.
    struct Dependency
    {
        std::optional<EdgeId> next_incoming_edge;
        std::optional<ChoiceId> choice_id;
    };

    ygg::SegmentedVector<SearchNode<Kind>>& m_nodes;
    const Predecessors<Kind>& m_predecessors;
    std::vector<StateProof> m_state_proofs;
    std::vector<Dependency> m_dependencies;
    ChoiceProofs m_choice_proofs;
    std::vector<ProgramStateView<Kind>> m_ready;

public:
    ProofPropagation(ygg::SegmentedVector<SearchNode<Kind>>& nodes, const Predecessors<Kind>& predecessors) : m_nodes(nodes), m_predecessors(predecessors) {}

    /// Each enabled Choose is a distinct requirement, including an empty one that can never succeed.
    ChoiceId add_choice(ProgramStateView<Kind> state)
    {
        auto& proof = proof_for(state);
        assert(!proof.enumeration_complete);
        ++proof.remaining_requirements;
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
            auto& source = proof_for(transition.source);
            assert(!source.enumeration_complete);
            ++source.remaining_requirements;
        }
        m_dependencies.push_back({ std::nullopt, choice_id });
        if (m_nodes[ygg::uint_t(transition.target.get_index())].status == SearchStatus::SUCCESS)
        {
            // Its success event may still be queued. Credit now WITHOUT linking it again.
            satisfy(edge);
        }
        else
        {
            auto& target = proof_for(transition.target);
            m_dependencies.back().next_incoming_edge = target.first_incoming_edge;
            target.first_incoming_edge = edge;
        }
    }

    /// All ordinary and Choose requirements selected by the execution mode have been declared.
    /// Later Choose bindings can still satisfy their already-declared requirement.
    void finish_enumeration(ProgramStateView<Kind> state)
    {
        proof_for(state).enumeration_complete = true;
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
            for (auto edge = proof_for(m_ready[i]).first_incoming_edge; edge; edge = m_dependencies[static_cast<ygg::uint_t>(*edge)].next_incoming_edge)
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
    StateProof& proof_for(ProgramStateView<Kind> state)
    {
        if (m_state_proofs.size() < m_nodes.size())
            m_state_proofs.resize(m_nodes.size());
        return m_state_proofs[ygg::uint_t(state.get_index())];
    }

    void satisfy(EdgeId edge)
    {
        if (const auto choice_id = m_dependencies[static_cast<ygg::uint_t>(edge)].choice_id; choice_id && !m_choice_proofs.satisfy(*choice_id))
            return;
        const auto source = m_predecessors[edge].source;
        auto& proof = proof_for(source);
        assert(proof.remaining_requirements != 0);
        --proof.remaining_requirements;
        try_prove(source);
    }

    void try_prove(ProgramStateView<Kind> state)
    {
        const auto& proof = proof_for(state);
        const auto& node = m_nodes[ygg::uint_t(state.get_index())];
        if (proof.enumeration_complete && proof.remaining_requirements == 0 && !node.is_open && !node.is_deadend && !node.is_unsolvable)
            succeed(state);
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
