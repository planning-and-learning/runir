#ifndef RUNIR_KR_PS_EXT_DETAIL_SEARCH_NODE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SEARCH_NODE_HPP_

#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <tyr/formalism/declarations.hpp>
#include <urd/repr/maps/index_map.hpp>
#include <urd/store.hpp>
#include <yggdrasil/formalism/row_view.hpp>

namespace runir::kr::ps::ext::detail
{

// Structural termination excludes back edges to ACTIVE states. Completed results
// are retained so shared continuations are evaluated only once.
enum class SearchStatus : std::uint8_t
{
    NEW,         // Not yet admitted to this search.
    DISCOVERED,  // Admitted, awaiting expansion.
    ACTIVE,
    SUCCESS,
    FAILURE,
};

template<tyr::TaskKind Kind>
struct SearchNode
{
    // First arrival is permanent; the initial state has no parent.
    std::optional<ProgramStateView<Kind>> parent_state = std::nullopt;
    std::optional<tyr::formalism::planning::ActionRowView> action = std::nullopt;
    // Number of non-singleton Choose bindings on the first-parent path.
    ygg::uint_t choice_depth = 0;
    // Maximum post-filter Choose binding count on the first-parent path.
    std::size_t choice_width = 0;
    SearchStatus status = SearchStatus::NEW;
    bool is_goal = false;
    bool is_unsolvable = false;
    bool is_deadend = false;
    bool is_open = false;
};

/// A search's nodes by state; a node stays where it is while others are added.
template<tyr::TaskKind Kind>
using SearchNodes = urd::Store<urd::map<urd::index<ProgramState<Kind>>, SearchNode<Kind>>, urd::dense, urd::stable_refs>;

template<tyr::TaskKind Kind>
SearchNode<Kind>& get_or_create_search_node(ProgramStateView<Kind> state, SearchNodes<Kind>& search_nodes)
{ return search_nodes[state.index()]; }

template<tyr::TaskKind Kind>
struct Predecessor
{
    ProgramStateView<Kind> source;
    ProgramStateView<Kind> target;
    std::optional<tyr::formalism::planning::ActionRowView> action;
    std::optional<RuleVariantView> rule;
};

}  // namespace runir::kr::ps::ext::detail

#endif
