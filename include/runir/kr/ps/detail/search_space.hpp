#ifndef RUNIR_KR_PS_DETAIL_SEARCH_SPACE_HPP_
#define RUNIR_KR_PS_DETAIL_SEARCH_SPACE_HPP_

#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <tyr/planning/plan.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>

namespace runir::kr::ps::detail
{

/// The plan to `goal`, replayed from the initial node: the actions of the nodes on goal's parent path, the root's
/// excluded. `parent(node)` is the node's parent, or nullptr at the root.
template<tyr::TaskKind Kind, typename Node, typename Parent>
tyr::planning::PackedPlan<Kind>
extract_total_ordered_plan(const Node& goal, Parent parent, const tyr::planning::PackedNode<Kind>& initial_node, runir::kr::TaskContext<Kind>& context)
{
    auto actions = urd::Store<urd::sequence<tyr::formalism::planning::ActionRowView>> {};
    for (const Node* node = &goal; const Node* next = parent(*node); node = next)
        if (node->action)
            actions.push_back(*node->action);
    std::ranges::reverse(actions);
    auto& search = *context.search_context;
    return tyr::planning::replay_plan<Kind>(initial_node, actions, *search.successor_generator, *search.state_repository, *search.axiom_evaluator);
}

}  // namespace runir::kr::ps::detail

#endif
