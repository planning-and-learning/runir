#ifndef RUNIR_KR_PS_EXT_DETAIL_SEARCH_STORAGE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SEARCH_STORAGE_HPP_

#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/predecessors.hpp"
#include "runir/kr/ps/ext/detail/transient_state.hpp"

#include <cassert>
#include <optional>
#include <utility>
#include <yggdrasil/containers/associative_containers.hpp>

namespace runir::kr::ps::ext::detail
{

/// ALL keeps state identity, first predecessors and every admitted transition.
template<tyr::TaskKind Kind>
struct InternedSearchStorage
{
    const ProgramSearchOptions<Kind>& options;
    ygg::SegmentedVector<SearchNode<Kind>> nodes;
    Predecessors<Kind> predecessors;
    ygg::uint_t num_reached = 0;

    explicit InternedSearchStorage(const ProgramSearchOptions<Kind>& options_) : options(options_) {}

    auto& node(ProgramStateView<Kind> state) { return get_or_create_search_node(state, nodes); }

    bool admit(ProgramStateView<Kind> state, auto&& classify)
    {
        auto& entry = node(state);
        if (entry.status != SearchStatus::NEW)
            return true;
        if (num_reached == options.max_num_states)
            return false;
        ++num_reached;
        entry.status = SearchStatus::DISCOVERED;
        const auto [goal, unsolvable] = classify(state);
        entry.is_goal = goal;
        entry.is_unsolvable = unsolvable;
        return true;
    }

    bool record_transition(ProgramStateView<Kind> source, const ProgramStep<Kind>& step, bool non_singleton, auto&& classify)
    {
        const auto target = step.get_target();
        auto& entry = node(target);
        const bool created = entry.status == SearchStatus::NEW;
        if (!admit(target, classify))
            return false;
        const auto& transition = step.get_state_transition();
        const auto action = transition ? std::optional(transition->action) : std::nullopt;
        predecessors.push_back({ source, target, action, step.rule });
        if (created)
        {
            entry.parent_state = source;
            entry.action = action;
            entry.choice_depth = node(source).choice_depth + ygg::uint_t(non_singleton);
        }
        return true;
    }

    std::optional<bool> completed(ProgramStateView<Kind> state)
    {
        const auto status = node(state).status;
        assert(status != SearchStatus::ACTIVE && "Structurally terminating execution cannot have a back edge.");
        if (status == SearchStatus::SUCCESS || status == SearchStatus::FAILURE)
            return status == SearchStatus::SUCCESS;
        return std::nullopt;
    }

    auto classify(ProgramStateView<Kind> state, auto&&)
    {
        const auto& entry = node(state);
        return std::pair(entry.is_goal, entry.is_unsolvable);
    }

    void record_flags(ProgramStateView<Kind> state, const auto& path)
    {
        auto& entry = node(state);
        entry.is_goal = path.is_goal;
        entry.is_unsolvable = path.is_unsolvable;
        entry.is_deadend = path.is_deadend;
        entry.is_open = path.is_open;
    }

    void memorize(ProgramStateView<Kind> state, bool) { node(state).status = SearchStatus::ACTIVE; }

    void complete(ProgramStateView<Kind> state, bool succeeded) { node(state).status = succeeded ? SearchStatus::SUCCESS : SearchStatus::FAILURE; }
};

/// NONE retains no memo entries; CHOICE retains only the combined result at a Choose source.
/// Every generated occurrence still consumes the search-work budget.
template<tyr::TaskKind Kind>
struct TransientSearchStorage
{
    const ProgramSearchOptions<Kind>& options;
    ygg::UnorderedMap<TransientProgramState<Kind>, SearchStatus> memo;
    ygg::uint_t num_reached = 0;

    explicit TransientSearchStorage(const ProgramSearchOptions<Kind>& options_) : options(options_) {}

    bool admit(const TransientProgramState<Kind>&, auto&&)
    {
        if (num_reached == options.max_num_states)
            return false;
        ++num_reached;
        return true;
    }

    bool record_transition(const TransientProgramState<Kind>&, const ProgramStep<Kind, TransientProgramState<Kind>>& step, bool, auto&& classify)
    {
        return admit(step.get_target(), classify);
    }

    std::optional<bool> completed(const TransientProgramState<Kind>& state)
    {
        if (options.state_memorization == StateMemorization::CHOICE)
            if (const auto found = memo.find(state); found != memo.end())
            {
                assert(found->second != SearchStatus::ACTIVE && "Structurally terminating execution cannot have a back edge.");
                return found->second == SearchStatus::SUCCESS;
            }
        return std::nullopt;
    }

    auto classify(const TransientProgramState<Kind>& state, auto&& classify) { return classify(state); }
    void record_flags(const TransientProgramState<Kind>&, const auto&) {}

    void memorize(const TransientProgramState<Kind>& state, bool has_choice)
    {
        if (options.state_memorization == StateMemorization::CHOICE && has_choice)
            memo.try_emplace(state, SearchStatus::ACTIVE);
    }

    void complete(const TransientProgramState<Kind>& state, bool succeeded)
    {
        if (const auto found = memo.find(state); found != memo.end())
            found->second = succeeded ? SearchStatus::SUCCESS : SearchStatus::FAILURE;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
