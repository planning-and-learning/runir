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

// ACTIVE states have a DFS frame; PENDING states have finished forward exploration
// but still depend on an unresolved proof. Neither status causes re-expansion.
// ProofPropagation can establish SUCCESS before or after the frame is popped.
// For example, A chooses B or Goal and B continues to A: B becomes PENDING,
// then A's Goal binding proves A and immediately notifies B through its incoming
// dependency. No replay is needed. If exploration and propagation both drain,
// an unresolved initial state has no finite proof and the search returns FAILURE.
enum class SearchStatus
{
    NEW,         // Not yet admitted to this search.
    DISCOVERED,  // Admitted, awaiting expansion.
    ACTIVE,
    PENDING,  // Forward exploration complete; proof dependencies remain unresolved.
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
