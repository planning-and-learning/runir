#ifndef RUNIR_KR_PS_BASE_DETAIL_SEARCH_NODE_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_SEARCH_NODE_HPP_

#include "runir/kr/ps/base/rule_view.hpp"

#include <optional>
#include <tyr/formalism/binding_view.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/state_index.hpp>
#include <yggdrasil/containers/segmented_vector.hpp>

namespace runir::kr::ps::base::detail
{

template<tyr::TaskKind Kind>
struct SearchNode
{
    std::optional<tyr::planning::PackedNode<Kind>> parent_node = std::nullopt;
    std::optional<tyr::formalism::planning::ActionBindingView> action = std::nullopt;
    bool is_goal = false;
    bool is_unsolvable = false;
    bool is_deadend = false;
    bool is_open = false;
};

template<tyr::TaskKind Kind>
SearchNode<Kind>& get_or_create_search_node(ygg::Index<tyr::planning::State<Kind>> state_index, ygg::SegmentedVector<SearchNode<Kind>>& search_nodes)
{
    while (ygg::uint_t(state_index) >= search_nodes.size())
        search_nodes.push_back(SearchNode<Kind> {});

    return search_nodes[ygg::uint_t(state_index)];
}

template<tyr::TaskKind Kind>
struct Predecessor
{
    ygg::Index<tyr::planning::State<Kind>> source;
    ygg::Index<tyr::planning::State<Kind>> target;
    tyr::formalism::planning::ActionBindingView action;
    RuleView rule;
};

}  // namespace runir::kr::ps::base::detail

#endif
