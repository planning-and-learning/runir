#ifndef RUNIR_KR_PS_EXT_DETAIL_SEARCH_PATH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SEARCH_PATH_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/ext/detail/pooled_shared_owner.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <optional>
#include <utility>

namespace runir::kr::ps::ext::detail
{

/// A retained path owns pooled states, not an explored-graph predecessor table.
/// Pending siblings and selected witnesses can outlive a DFS frame; discarded branches release their states.
template<tyr::TaskKind Kind, ProgramStateViewConcept<Kind> S>
struct SearchPath
{
    std::optional<S> state;
    PooledSharedOwner<SearchPath<Kind, S>> parent;
    std::optional<datasets::StateGraphEdgeLabel> transition;
    std::optional<RuleVariantView> rule;
    ygg::uint_t choice_depth = 0;
    bool is_goal = false;
    bool is_unsolvable = false;
    bool is_deadend = false;
    bool is_open = false;

    void initialize(S state_,
                    PooledSharedOwner<SearchPath<Kind, S>> parent_,
                    std::optional<datasets::StateGraphEdgeLabel> transition_,
                    std::optional<RuleVariantView> rule_,
                    ygg::uint_t choice_depth_)
    {
        state = std::move(state_);
        parent = std::move(parent_);
        transition = transition_;
        rule = rule_;
        choice_depth = choice_depth_;
        is_goal = is_unsolvable = is_deadend = is_open = false;
    }

    void release_owners() noexcept
    {
        state.reset();
        // Releasing a long witness must not recursively destroy the entire parent chain.
        while (parent && parent.ref_count() == 1)
        {
            auto previous = std::move(parent);
            parent = std::move(previous->parent);
        }
        parent.reset();
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
