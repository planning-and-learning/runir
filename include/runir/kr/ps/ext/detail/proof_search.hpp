#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/proof_graph.hpp"
#include "runir/kr/ps/ext/detail/search_path.hpp"
#include "runir/kr/ps/ext/detail/search_space.hpp"
#include "runir/kr/ps/ext/detail/search_storage.hpp"
#include "runir/kr/ps/ext/dl/structural_termination.hpp"
#include "runir/kr/ps/ext/program_executor.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"
#include "runir/kr/ps/unsolvability.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <tuple>
#include <type_traits>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <variant>
#include <vector>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::ps::ext
{
namespace detail
{

/// Evaluate terminating executions in postorder. The storage policy controls state
/// admission, memoization and graph retention; AND/OR traversal is identical in every mode.
template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> ExecutionStorage, typename Expander, typename Storage, typename Unsolvability>
ProgramProofStatus depth_first_search(Expander& expander,
                                      const tyr::planning::PackedStateView<Kind>& initial_state,
                                      const typename ExecutionStorage::StoredState& initial,
                                      const ProgramSearchOptions<Kind>& options,
                                      Unsolvability& classifier,
                                      Storage& storage,
                                      ProgramSearchStatistics& statistics,
                                      ygg::SharedObjectPool<SearchPath<Kind, ExecutionStorage>>& path_pool,
                                      ygg::SharedObjectPoolPtr<SearchPath<Kind, ExecutionStorage>>& first_goal,
                                      ygg::SharedObjectPoolPtr<SearchPath<Kind, ExecutionStorage>>& witness)
{
    using Step = ProgramStep<Kind, ExecutionStorage>;
    using PathPtr = ygg::SharedObjectPoolPtr<SearchPath<Kind, ExecutionStorage>>;
    struct Frame
    {
        PathPtr path;
        std::size_t successors_begin;
        std::size_t choices_begin;
        std::optional<ProgramStateView<Kind>> memo_state;
        bool choice_child = false;
        bool succeeded = true;
    };

    const auto& task_context = expander.get_task_context();
    auto& search = *task_context->search_context;
    const auto stopwatch = options.max_time ? std::optional<ygg::CountdownWatch>(*options.max_time) : std::nullopt;
    const auto initial_planning_state = initial_state.unpack();
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*search.task);
    const bool static_goal = goal_strategy.is_static_goal_satisfied(*search.task);
    auto stack = std::vector<Frame> {};
    auto successors = std::vector<Step> {};
    auto choices = std::vector<std::variant<Choice<runir::kr::dl::ConceptTag>, Choice<runir::kr::dl::RoleTag>>> {};
    auto status = ProgramProofStatus::SUCCESS;
    auto first_failure = PathPtr {};
    auto next = PathPtr {};
    auto completed = std::optional<bool> {};
    const auto out_of_time = [&] { return stopwatch && stopwatch->has_finished(); };
    const auto classify = [&](ProgramStateViewConcept<Kind> auto state)
    {
        const auto planning_state = state.get_state();
        const bool goal = static_goal && goal_strategy.is_dynamic_goal_satisfied(initial_planning_state, planning_state.get_state_builder());
        return std::pair(goal, !goal && classifier.is_unsolvable(planning_state));
    };
    const auto record_transition = [&](ProgramStateViewConcept<Kind> auto source, const Step& step, std::size_t choice_width)
    {
        const auto& transition = step.get_state_transition();
        const auto action = transition ? std::optional(transition->action) : std::nullopt;
        return storage.record_transition(source, expander.view(step.target), action, step.rule, choice_width, classify);
    };
    const auto make_path = [&](typename ExecutionStorage::StoredState state,
                               PathPtr parent,
                               std::optional<datasets::StateGraphEdgeLabel> transition,
                               std::optional<RuleVariantView> rule,
                               std::size_t choice_width)
    {
        const auto depth = parent ? parent->choice_depth + ygg::uint_t(choice_width > 1) : 0;
        const auto width = parent ? std::max(parent->choice_width, choice_width) : 0;
        return path_pool.get_or_allocate(std::move(state), std::move(parent), transition, rule, depth, width);
    };
    if (storage.admit(expander.view(initial), classify))
        next = make_path(initial, {}, {}, {}, 0);
    else
        status = ProgramProofStatus::OUT_OF_STATES;

    while (next || !stack.empty())
    {
        witness = next ? next : stack.back().path;
        if (out_of_time())
        {
            status = ProgramProofStatus::OUT_OF_TIME;
            break;
        }
        if (next)
        {
            auto path = std::move(next);
            const auto state = expander.view(*path->state);
            const auto memo_state = storage.initial_memo_state(state);
            if (memo_state)
            {
                if (const auto cached = storage.completed(*memo_state))
                {
                    completed = *cached;
                    continue;
                }
                storage.memorize(*memo_state);
            }
            std::tie(path->is_goal, path->is_unsolvable) = storage.classify(state, classify);
            if (path->is_goal || path->is_unsolvable)
            {
                if (path->is_goal && !first_goal)
                    first_goal = path;
                path->is_deadend = path->is_unsolvable;
                if (path->is_unsolvable && !first_failure)
                    first_failure = path;
                storage.record_flags(state, *path);
                if (memo_state)
                    storage.complete(*memo_state, path->is_goal);
                completed = path->is_goal;
                continue;
            }
            ++statistics.num_expanded;
            auto frame = Frame { std::move(path), successors.size(), choices.size(), memo_state };
            auto limit = std::optional<ProgramProofStatus> {};
            expander.for_each_successor(
                state,
                statistics,
                [&](auto expansion)
                {
                    if (out_of_time())
                    {
                        limit = ProgramProofStatus::OUT_OF_TIME;
                        return false;
                    }
                    if constexpr (std::same_as<decltype(expansion), Step>)
                    {
                        if (expansion.status == ProgramOutcome::APPLIED || expansion.status == ProgramOutcome::RESTORED_CALLER)
                        {
                            if (!record_transition(state, expansion, 0))
                                limit = ProgramProofStatus::OUT_OF_STATES;
                            else
                                successors.push_back(std::move(expansion));
                        }
                        else
                        {
                            frame.path->is_open = true;
                            frame.succeeded = false;
                            if (!first_failure)
                                first_failure = frame.path;
                        }
                    }
                    else
                    {
                        if (!frame.memo_state)
                        {
                            frame.memo_state = storage.choice_memo_state([&] { return expander.materialize(state); });
                            if (frame.memo_state)
                            {
                                if (const auto cached = storage.completed(*frame.memo_state))
                                {
                                    completed = *cached;
                                    return false;
                                }
                                storage.memorize(*frame.memo_state);
                            }
                        }
                        if (expansion.exhausted())
                        {
                            frame.path->is_deadend = true;
                            if (!first_failure)
                                first_failure = frame.path;
                        }
                        choices.emplace_back(std::move(expansion));
                    }
                    return !limit && options.universal;
                },
                out_of_time);
            storage.record_flags(state, *frame.path);
            if (out_of_time() && !limit)
                limit = ProgramProofStatus::OUT_OF_TIME;
            if (limit)
            {
                status = *limit;
                break;
            }
            if (completed)
            {
                // A choice-source memo covers every obligation, including ordinary
                // successors emitted before the first Choose revealed its identity.
                successors.erase(successors.begin() + frame.successors_begin, successors.end());
                choices.erase(choices.begin() + frame.choices_begin, choices.end());
                continue;
            }
            stack.push_back(std::move(frame));
            continue;
        }

        auto& frame = stack.back();
        const auto state = expander.view(*frame.path->state);
        if (completed)
        {
            if (frame.choice_child)
            {
                if (*completed)
                    choices.pop_back();
            }
            else
                frame.succeeded &= *completed;
            completed.reset();
        }

        // Each frame owns a suffix of the shared buffers. Children append above pending
        // siblings and consume their suffix before returning; only offsets survive descent.
        if (successors.size() != frame.successors_begin)
        {
            auto step = std::move(successors.back());
            successors.pop_back();
            frame.choice_child = false;
            next = make_path(step.get_target(), frame.path, step.get_state_transition(), step.rule, 0);
            continue;
        }
        if (choices.size() != frame.choices_begin)
        {
            auto step = std::optional<Step> {};
            std::size_t choice_width = 0;
            std::visit(
                [&](auto& choice)
                {
                    if (!choice.exhausted())
                    {
                        choice_width = choice.count();
                        step = expander.apply_choice(state, choice, statistics);
                        choice.advance();
                    }
                },
                choices.back());
            if (!step)
            {
                frame.succeeded = false;
                choices.pop_back();
                continue;
            }
            if (!record_transition(state, *step, choice_width))
            {
                status = ProgramProofStatus::OUT_OF_STATES;
                break;
            }
            frame.choice_child = true;
            next = make_path(step->get_target(), frame.path, step->get_state_transition(), step->rule, choice_width);
            continue;
        }
        if (frame.memo_state)
            storage.complete(*frame.memo_state, frame.succeeded);
        completed = frame.succeeded;
        stack.pop_back();
    }
    if (completed && stack.empty())
    {
        status = *completed ? ProgramProofStatus::SUCCESS : ProgramProofStatus::FAILURE;
        witness = *completed ? first_goal : first_failure;
    }
    return status;
}

/// Own the search lifetime, then construct its graph and optional execution plan.
template<tyr::TaskKind Kind, StateMemorization Memorization, typename Expander, typename Unsolvability>
ProgramProofResults<Kind>
find_solution(Expander& expander, SearchStorage<Kind, Memorization>& storage, const ProgramSearchOptions<Kind>& options, Unsolvability& classifier)
{
    const auto& task_context = expander.get_task_context();
    auto& search = *task_context->search_context;
    const auto initial_node = search.successor_generator->get_packed_initial_node(*search.state_repository, *search.axiom_evaluator);
    const auto initial = expander.initial_state(initial_node.get_state().unpack());
    using ExecutionStorage = typename Expander::StorageType;
    using PathPtr = ygg::SharedObjectPoolPtr<SearchPath<Kind, ExecutionStorage>>;
    auto statistics = ProgramSearchStatistics {};
    auto path_pool = ygg::SharedObjectPool<SearchPath<Kind, ExecutionStorage>> {};
    auto first_goal = PathPtr {};
    auto witness = PathPtr {};
    const auto status =
        depth_first_search<Kind>(expander, initial_node.get_state(), initial, options, classifier, storage, statistics, path_pool, first_goal, witness);

    auto result = ProgramProofResults<Kind> {};
    result.task_context_owner = task_context;
    result.status = status;
    result.statistics = statistics;
    if constexpr (Memorization != StateMemorization::ALL)
    {
        if constexpr (Memorization == StateMemorization::CHOICE)
            storage.memo.clear();
        build_witness_graph(result, expander, witness);
        if (status == ProgramProofStatus::SUCCESS && first_goal)
        {
            result.statistics.choice_depth = first_goal->choice_depth;
            result.statistics.choice_width = first_goal->choice_width;
            if (!options.universal)
            {
                auto actions = std::vector<tyr::formalism::planning::ActionBindingView> {};
                for (auto path = first_goal; path; path = path->parent)
                    if (path->transition)
                        actions.push_back(path->transition->action);
                std::ranges::reverse(actions);
                result.plan = extract_total_ordered_plan<Kind>(actions, initial_node, *task_context);
            }
        }
    }
    else
    {
        build_proof_graph(result, storage.nodes, storage.predecessors, storage.num_reached ? std::optional(initial) : std::nullopt);
        if (status == ProgramProofStatus::SUCCESS && first_goal)
        {
            const auto goal = *first_goal->state;
            result.statistics.choice_depth = storage.node(goal).choice_depth;
            result.statistics.choice_width = storage.node(goal).choice_width;
            if (!options.universal)
                result.plan = extract_total_ordered_plan(goal, storage.nodes, initial_node, *task_context);
        }
    }
    return result;
}

}  // namespace detail

template<tyr::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context_owner,
                   ProgramView program,
                   const ProgramSearchOptions<Kind>& options) -> ProgramProofResults<Kind>
{
    if (!dl::structural_termination(program).is_terminating())
        throw std::invalid_argument("Ext find_solution requires a structurally terminating program.");

    const auto execute = [&](auto& expander, auto& storage)
    {
        if (options.classifier)
        {
            auto classifier = ClassifierUnsolvability<Kind>(*task_context_owner, *options.classifier);
            return detail::find_solution<Kind>(expander, storage, options, classifier);
        }
        auto classifier = NoUnsolvability {};
        return detail::find_solution<Kind>(expander, storage, options, classifier);
    };
    // Validate the task and program before constructing a classifier that borrows the task context.
    switch (options.state_memorization)
    {
        case StateMemorization::ALL:
        {
            auto expander = SuccessorExpander<Kind>(task_context_owner, program);
            auto storage = detail::SearchStorage<Kind, StateMemorization::ALL>(options);
            return execute(expander, storage);
        }
        case StateMemorization::NONE:
        {
            auto expander = SuccessorExpander<Kind, TransientExecutionStorage<Kind>>(task_context_owner, program);
            auto storage = detail::SearchStorage<Kind, StateMemorization::NONE>(options);
            return execute(expander, storage);
        }
        case StateMemorization::CHOICE:
        {
            auto expander = SuccessorExpander<Kind, TransientExecutionStorage<Kind>>(task_context_owner, program);
            auto storage = detail::SearchStorage<Kind, StateMemorization::CHOICE>(options);
            return execute(expander, storage);
        }
    }
    throw std::invalid_argument("Invalid Ext state memorization mode.");
}

}  // namespace runir::kr::ps::ext

#endif
