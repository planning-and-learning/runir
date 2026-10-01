#ifndef RUNIR_KR_PS_EXT_DETAIL_SEARCH_STORAGE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SEARCH_STORAGE_HPP_

#include "runir/kr/ps/ext/detail/search_node.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"

#include <cassert>
#include <optional>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind, StateMemorization Memorization>
struct SearchStorage;

/// ALL keeps state identity, first predecessors and every admitted transition.
template<tyr::TaskKind Kind>
struct SearchStorage<Kind, StateMemorization::ALL>
{
    const ProgramSearchOptions<Kind>& options;
    ygg::SegmentedVector<SearchNode<Kind>> nodes;
    // Retains each admitted transition in discovery order, including parallel edges.
    std::vector<Predecessor<Kind>> predecessors;
    ygg::uint_t num_reached = 0;

    explicit SearchStorage(const ProgramSearchOptions<Kind>& options_) : options(options_) {}

    auto& node(ProgramStateView<Kind> state) { return get_or_create_search_node(state, nodes); }

    auto initial_memo_state(ProgramStateView<Kind> state) { return std::optional(state); }
    std::optional<ProgramStateView<Kind>> choice_memo_state(auto&&) { return std::nullopt; }

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

    bool record_transition(ProgramStateView<Kind> source,
                           ProgramStateView<Kind> target,
                           std::optional<tyr::formalism::planning::ActionBindingView> action,
                           std::optional<RuleVariantView> rule,
                           bool non_singleton,
                           auto&& classify)
    {
        auto& entry = node(target);
        const bool created = entry.status == SearchStatus::NEW;
        if (!admit(target, classify))
            return false;
        predecessors.push_back({ source, target, action, rule });
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
        entry.is_deadend = path.is_deadend;
        entry.is_open = path.is_open;
    }

    void memorize(ProgramStateView<Kind> state) { node(state).status = SearchStatus::ACTIVE; }

    void complete(ProgramStateView<Kind> state, bool succeeded) { node(state).status = succeeded ? SearchStatus::SUCCESS : SearchStatus::FAILURE; }
};

/// NONE retains no memo entries or memo table.
/// Every generated occurrence still consumes the search-work budget.
template<tyr::TaskKind Kind>
struct SearchStorage<Kind, StateMemorization::NONE>
{
    const ProgramSearchOptions<Kind>& options;
    ygg::uint_t num_reached = 0;

    explicit SearchStorage(const ProgramSearchOptions<Kind>& options_) : options(options_) {}

    std::optional<ProgramStateView<Kind>> initial_memo_state(BuilderProgramStateView<Kind>) { return std::nullopt; }
    std::optional<ProgramStateView<Kind>> choice_memo_state(auto&&) { return std::nullopt; }

    bool admit(BuilderProgramStateView<Kind>, auto&&)
    {
        if (num_reached == options.max_num_states)
            return false;
        ++num_reached;
        return true;
    }

    bool record_transition(BuilderProgramStateView<Kind>,
                           BuilderProgramStateView<Kind> target,
                           std::optional<tyr::formalism::planning::ActionBindingView>,
                           std::optional<RuleVariantView>,
                           bool,
                           auto&& classify)
    {
        return admit(target, classify);
    }

    std::optional<bool> completed(ProgramStateView<Kind>) { return std::nullopt; }

    auto classify(BuilderProgramStateView<Kind> state, auto&& classify) { return classify(state); }
    void record_flags(BuilderProgramStateView<Kind>, const auto&) {}
    void memorize(ProgramStateView<Kind>) {}
    void complete(ProgramStateView<Kind>, bool) {}
};

/// CHOICE interns each Choose source and keys its combined result by repository identity.
/// Other state occurrences share NONE's pooled storage and work accounting.
template<tyr::TaskKind Kind>
struct SearchStorage<Kind, StateMemorization::CHOICE> : SearchStorage<Kind, StateMemorization::NONE>
{
    ygg::UnorderedMap<ProgramStateView<Kind>, SearchStatus> memo;

    explicit SearchStorage(const ProgramSearchOptions<Kind>& options_) : SearchStorage<Kind, StateMemorization::NONE>(options_) {}

    ProgramStateView<Kind> choice_memo_state(auto&& materialize) { return materialize(); }

    std::optional<bool> completed(ProgramStateView<Kind> state)
    {
        if (const auto found = memo.find(state); found != memo.end())
        {
            assert(found->second != SearchStatus::ACTIVE && "Structurally terminating execution cannot have a back edge.");
            return found->second == SearchStatus::SUCCESS;
        }
        return std::nullopt;
    }

    void memorize(ProgramStateView<Kind> state) { memo.try_emplace(state, SearchStatus::ACTIVE); }

    void complete(ProgramStateView<Kind> state, bool succeeded) { memo.at(state) = succeeded ? SearchStatus::SUCCESS : SearchStatus::FAILURE; }
};

}  // namespace runir::kr::ps::ext::detail

#endif
