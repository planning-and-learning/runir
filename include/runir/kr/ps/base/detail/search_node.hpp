#ifndef RUNIR_KR_PS_BASE_DETAIL_SEARCH_NODE_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_SEARCH_NODE_HPP_

#include "runir/kr/ps/base/rule_view.hpp"

#include <cstddef>
#include <limits>
#include <optional>
#include <tuple>
#include <tyr/formalism/binding_view.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/planning/state_index.hpp>
#include <yggdrasil/containers/segmented_vector.hpp>
#include <yggdrasil/semantics/comparison.hpp>

namespace runir::kr::ps::base::detail
{

template<tyr::TaskKind Kind>
using StateIndex = ygg::Index<tyr::planning::State<Kind>>;

template<tyr::TaskKind Kind>
struct SearchNode
{
    static constexpr auto unreached = std::numeric_limits<std::size_t>::max();

    ygg::Index<tyr::planning::State<Kind>> parent_state = ygg::Index<tyr::planning::State<Kind>>::max();
    std::optional<tyr::formalism::planning::ActionBindingView> action = std::nullopt;
    ygg::float_t metric = 0;
    std::size_t discovery_order = unreached;
    bool visited = false;
    bool is_goal = false;
    bool is_unsolvable = false;
    bool has_successor = false;
    bool boundary = false;
};

template<tyr::TaskKind Kind>
SearchNode<Kind>& get_or_create_search_node(StateIndex<Kind> state_index, ygg::SegmentedVector<SearchNode<Kind>>& search_nodes)
{
    while (ygg::uint_t(state_index) >= search_nodes.size())
        search_nodes.push_back(SearchNode<Kind> {});

    return search_nodes[ygg::uint_t(state_index)];
}

template<tyr::TaskKind Kind>
struct Predecessor : ygg::comparison::Mixin<Predecessor<Kind>>
{
    StateIndex<Kind> source;
    StateIndex<Kind> target;
    tyr::formalism::planning::ActionBindingView action;
    ygg::float_t cost;
    RuleView rule;
    std::size_t order;

    Predecessor(StateIndex<Kind> source_,
                StateIndex<Kind> target_,
                tyr::formalism::planning::ActionBindingView action_,
                ygg::float_t cost_,
                RuleView rule_,
                std::size_t order_) :
        source(source_),
        target(target_),
        action(action_),
        cost(cost_),
        rule(rule_),
        order(order_)
    {
    }

    auto identifying_members() const noexcept { return std::tie(source, target, action, cost, rule); }
};

}  // namespace runir::kr::ps::base::detail

#endif
