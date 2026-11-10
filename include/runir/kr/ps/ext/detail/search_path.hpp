#ifndef RUNIR_KR_PS_EXT_DETAIL_SEARCH_PATH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SEARCH_PATH_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <cstddef>
#include <optional>
#include <utility>
#include <urd/repr/slots/slots.hpp>
#include <urd/store.hpp>

namespace runir::kr::ps::ext::detail
{

/// A retained path owns pooled states, not an explored-graph predecessor table.
/// Pending siblings and selected witnesses can outlive a DFS frame; discarded branches release their states.
/// The search owns the pool until graph and plan construction have released every path.
template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> Storage>
struct SearchPath
{
    using StoredState = StorageStoredState<Storage>;
    std::optional<StoredState> state;
    std::shared_ptr<SearchPath<Kind, Storage>> parent;
    std::optional<datasets::StateGraphEdgeLabel> transition;
    std::optional<RuleVariantView> rule;
    ygg::uint_t choice_depth = 0;
    std::size_t choice_width = 0;
    bool is_goal = false;
    bool is_unsolvable = false;
    bool is_deadend = false;
    bool is_open = false;

    void initialize(StoredState state_,
                    std::shared_ptr<SearchPath<Kind, Storage>> parent_,
                    std::optional<datasets::StateGraphEdgeLabel> transition_,
                    std::optional<RuleVariantView> rule_,
                    ygg::uint_t choice_depth_,
                    std::size_t choice_width_)
    {
        state = std::move(state_);
        parent = std::move(parent_);
        transition = transition_;
        rule = rule_;
        choice_depth = choice_depth_;
        choice_width = choice_width_;
        is_goal = is_unsolvable = is_deadend = is_open = false;
    }

    void on_pool_release() noexcept
    {
        state.reset();
        // Releasing a long witness must not recursively destroy the entire parent chain.
        while (parent && parent.use_count() == 1)
        {
            auto previous = std::move(parent);
            parent = std::move(previous->parent);
        }
        parent = {};
    }
};

}  // namespace runir::kr::ps::ext::detail

/// A path given back to its pool lets go of its state and its parents.
template<tyr::TaskKind Kind, runir::kr::ps::ext::ExecutionStorageConcept<Kind> Storage>
struct urd::slot_release<runir::kr::ps::ext::detail::SearchPath<Kind, Storage>>
{
    static void release(runir::kr::ps::ext::detail::SearchPath<Kind, Storage>& path) noexcept { path.on_pool_release(); }
};

#endif
