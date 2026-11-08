#ifndef RUNIR_KR_PS_BASE_DETAIL_SEARCH_NODE_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_SEARCH_NODE_HPP_

#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/rule_view.hpp"

#include <optional>
#include <tyr/formalism/declarations.hpp>
#include <yggdrasil/formalism/row_view.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/planning/state_repository.hpp>
#include <urd/repr/sequences/segmented.hpp>
#include <urd/store.hpp>

namespace runir::kr::ps::base::detail
{

template<tyr::TaskKind Kind>
struct SearchNode
{
    std::optional<tyr::planning::PackedStateView<Kind>> parent_state = std::nullopt;
    std::optional<tyr::formalism::planning::ActionRowView> action = std::nullopt;
    bool is_goal = false;
    bool is_unsolvable = false;
    bool is_deadend = false;
    bool is_open = false;
};

template<tyr::TaskKind Kind>
SearchNode<Kind>& get_or_create_search_node(const tyr::planning::PackedStateView<Kind>& state, urd::Store<urd::sequence<SearchNode<Kind>>, urd::stable_refs>& search_nodes)
{
    const auto index = ygg::uint_t(state.index().value);
    while (index >= search_nodes.size())
        search_nodes.push_back(SearchNode<Kind> {});

    return search_nodes[index];
}

template<tyr::TaskKind Kind>
struct Predecessor
{
    tyr::planning::PackedStateView<Kind> source;
    tyr::planning::PackedStateView<Kind> target;
    tyr::formalism::planning::ActionRowView action;
    RuleView rule;
};

}  // namespace runir::kr::ps::base::detail

#endif
