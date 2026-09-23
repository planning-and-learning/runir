#ifndef RUNIR_KR_PS_EXT_DETAIL_SEARCH_NODE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SEARCH_NODE_HPP_

#include "runir/kr/ps/ext/execution_index.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <cstddef>
#include <optional>
#include <tyr/formalism/binding_view.hpp>
#include <tyr/planning/node.hpp>
#include <yggdrasil/containers/segmented_vector.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
struct SearchNode
{
    // First arrival is permanent. The initial state is tracked separately.
    ygg::Index<ProgramState<Kind>> parent_state = ygg::Index<ProgramState<Kind>>::max();
    std::optional<tyr::planning::PackedLabeledNode<Kind>> planning_successor = std::nullopt;
    // Number of non-singleton Choose bindings on the first-parent path.
    ygg::uint_t choice_depth = 0;
    bool is_goal = false;
    bool is_unsolvable = false;
    bool is_deadend = false;
    bool is_open = false;
};

template<tyr::TaskKind Kind>
SearchNode<Kind>& get_or_create_search_node(ygg::Index<ProgramState<Kind>> state_index, ygg::SegmentedVector<SearchNode<Kind>>& search_nodes)
{
    while (ygg::uint_t(state_index) >= search_nodes.size())
        search_nodes.push_back(SearchNode<Kind> {});

    return search_nodes[ygg::uint_t(state_index)];
}

template<tyr::TaskKind Kind>
struct Predecessor
{
    ygg::Index<ProgramState<Kind>> source;
    ygg::Index<ProgramState<Kind>> target;
    std::optional<tyr::formalism::planning::ActionBindingView> action;
    std::optional<RuleVariantView> rule;
    // Bindings of the same Choose rule at the same state share one existential obligation.
    std::optional<std::size_t> choice = std::nullopt;
};

}  // namespace runir::kr::ps::ext::detail

#endif
