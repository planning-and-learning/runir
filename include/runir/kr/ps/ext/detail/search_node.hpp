#ifndef RUNIR_KR_PS_EXT_DETAIL_SEARCH_NODE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SEARCH_NODE_HPP_

#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <cstddef>
#include <optional>
#include <tyr/formalism/binding_view.hpp>
#include <yggdrasil/containers/segmented_vector.hpp>

namespace runir::kr::ps::ext::detail
{

// Reaching an ACTIVE ancestor returns PENDING to the caller without changing
// the ancestor's status. A frame stores PENDING when it unwinds with an unresolved
// cyclic dependency instead of an established success or definite failure.
//
// For example, A chooses B or Goal, and B continues to A:
// 1. A and then B become ACTIVE.
// 2. B reaches active A and receives PENDING; A remains ACTIVE.
// 3. B unwinds as PENDING because its continuation is unresolved.
// 4. A tries Goal and becomes SUCCESS.
// A later visit to B can now succeed through A. It reconsiders recorded
// continuations without generating B's successors again. Therefore PENDING
// cannot be cached as FAILURE; encountering a duplicate alone does not change
// that state's status.
enum class SearchStatus
{
    NEW,         // Not yet admitted to this search.
    DISCOVERED,  // Admitted, awaiting expansion.
    ACTIVE,
    PENDING,  // Depends on an active ancestor; not a cached failure.
    SUCCESS,
    FAILURE,
};

template<tyr::TaskKind Kind>
struct SearchNode
{
    // First arrival is permanent; the initial state has no parent.
    std::optional<ProgramStateView<Kind>> parent_state = std::nullopt;
    std::optional<tyr::formalism::planning::ActionBindingView> action = std::nullopt;
    // Number of non-singleton Choose bindings on the first-parent path.
    ygg::uint_t choice_depth = 0;
    SearchStatus status = SearchStatus::NEW;
    bool is_goal = false;
    bool is_unsolvable = false;
    bool is_deadend = false;
    bool is_open = false;
};

template<tyr::TaskKind Kind>
SearchNode<Kind>& get_or_create_search_node(ProgramStateView<Kind> state, ygg::SegmentedVector<SearchNode<Kind>>& search_nodes)
{
    const auto index = ygg::uint_t(state.get_index());
    while (index >= search_nodes.size())
        search_nodes.push_back(SearchNode<Kind> {});

    return search_nodes[index];
}

template<tyr::TaskKind Kind>
struct Predecessor
{
    ProgramStateView<Kind> source;
    ProgramStateView<Kind> target;
    std::optional<tyr::formalism::planning::ActionBindingView> action;
    std::optional<RuleVariantView> rule;
};

}  // namespace runir::kr::ps::ext::detail

#endif
