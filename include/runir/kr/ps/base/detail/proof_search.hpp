#ifndef RUNIR_KR_PS_BASE_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/base/detail/proof_graph.hpp"
#include "runir/kr/ps/base/detail/search_space.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/sketch_executor.hpp"
#include "runir/kr/ps/base/successor_expander.hpp"
#include "runir/kr/ps/unsolvability.hpp"
#include "runir/kr/task_context.hpp"

#include <optional>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/containers/segmented_vector.hpp>
#include <yggdrasil/core/chrono.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::base
{
namespace detail
{

/// Traverse permitted successors in depth-first order, expanding each reached state once.
/// Report the first visited goal in ordinary execution; universal execution visits every permitted continuation.
template<tyr::TaskKind Kind, typename Unsolvability>
SketchProofStatus depth_first_search(runir::kr::TaskContext<Kind>& task_context,
                                     SuccessorExpander<Kind>& expander,
                                     const tyr::planning::PackedNode<Kind>& initial_node,
                                     const SketchSearchOptions<Kind>& options,
                                     Unsolvability& classifier,
                                     ygg::SegmentedVector<SearchNode<Kind>>& nodes,
                                     std::vector<Predecessor<Kind>>& predecessors,
                                     SketchSearchStatistics& statistics,
                                     std::optional<tyr::planning::PackedStateView<Kind>>& goal)
{
    const auto& search_context = *task_context.search_context;
    auto num_reached = std::size_t(0);
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*search_context.task);
    const auto initial_state = initial_node.get_state().unpack();
    const auto static_goal_satisfied = goal_strategy.is_static_goal_satisfied(*search_context.task);
    const auto stopwatch = options.max_time ? std::optional<ygg::CountdownWatch>(*options.max_time) : std::nullopt;

    const auto out_of_time = [&]() { return stopwatch && stopwatch->has_finished(); };

    const auto is_goal = [&](const tyr::planning::StateView<Kind>& state)
    { return static_goal_satisfied && goal_strategy.is_dynamic_goal_satisfied(initial_state, state); };

    const auto get_or_create_state = [&](const tyr::planning::StateView<Kind>& state) -> std::optional<bool>
    {
        const auto position = std::size_t(ygg::uint_t(state.get_index()));
        if ((num_reached > 0 && state == initial_state) || (position < nodes.size() && nodes[position].parent_state))
            return false;
        if (num_reached >= options.max_num_states)
            return std::nullopt;
        auto& node = detail::get_or_create_search_node(state.pack(), nodes);
        ++num_reached;
        node.is_goal = is_goal(state);
        node.is_unsolvable = !node.is_goal && classifier.is_unsolvable(state);
        return true;
    };

    const auto initial = get_or_create_state(initial_state);
    if (!initial)
        return SketchProofStatus::OUT_OF_STATES;
    auto open = tyr::planning::PackedNodeList<Kind> { initial_node };

    while (!open.empty())
    {
        if (out_of_time())
            return SketchProofStatus::OUT_OF_TIME;

        const auto packed_node = std::move(open.back());
        open.pop_back();
        const auto& source = packed_node.get_state();
        auto& source_node = detail::get_or_create_search_node(source, nodes);
        if (source_node.is_goal)
        {
            if (!options.universal)
            {
                goal = source;
                return SketchProofStatus::SUCCESS;
            }
            continue;
        }
        if (source_node.is_unsolvable)
        {
            source_node.is_deadend = true;
            continue;
        }

        ++statistics.num_expanded;
        const auto node = packed_node.unpack();
        auto expansion_status = SketchProofStatus::SUCCESS;
        bool is_open = true;
        const auto accept_successor = [&](const tyr::planning::LabeledNode<Kind>& successor, RuleView rule)
        {
            if (out_of_time())
            {
                expansion_status = SketchProofStatus::OUT_OF_TIME;
                return false;
            }
            const auto target_result = get_or_create_state(successor.node.get_state());
            if (!target_result)
            {
                expansion_status = SketchProofStatus::OUT_OF_STATES;
                return false;
            }
            const auto target = successor.node.get_state().pack();
            predecessors.push_back({ source, target, successor.label, rule });
            if (*target_result)
            {
                auto& target_node = detail::get_or_create_search_node(target, nodes);
                target_node.parent_state = source;
                target_node.action = successor.label;
                open.push_back(successor.node.pack());
            }
            is_open = false;
            return options.universal;
        };

        expander.for_each_successor(node, statistics, accept_successor, out_of_time);
        if (expansion_status != SketchProofStatus::SUCCESS)
            return expansion_status;
        if (out_of_time())
            return SketchProofStatus::OUT_OF_TIME;
        source_node.is_open = is_open;
    }

    return SketchProofStatus::SUCCESS;
}

/// Own the search lifetime, then construct its graph and optional execution plan.
template<tyr::TaskKind Kind, typename Unsolvability>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context_owner, SketchView sketch, const SketchSearchOptions<Kind>& options, Unsolvability& classifier)
    -> SketchProofResults<Kind>
{
    auto& task_context = *task_context_owner;
    const auto& search_context = *task_context.search_context;
    auto expander = SuccessorExpander<Kind>(task_context, sketch);
    const auto initial_node = search_context.successor_generator->get_packed_initial_node(*search_context.state_repository, *search_context.axiom_evaluator);
    auto nodes = ygg::SegmentedVector<SearchNode<Kind>> {};
    auto predecessors = std::vector<Predecessor<Kind>> {};
    auto statistics = SketchSearchStatistics {};
    auto goal = std::optional<tyr::planning::PackedStateView<Kind>> {};

    const auto status = depth_first_search(task_context, expander, initial_node, options, classifier, nodes, predecessors, statistics, goal);

    auto result = SketchProofResults<Kind> {};
    result.task_context_owner = task_context_owner;
    result.status = status;
    result.statistics = statistics;
    // Nodes remain empty only when the state limit prevents admitting the initial state.
    build_proof_graph<Kind>(result, nodes, predecessors, nodes.empty() ? std::nullopt : std::optional(initial_node.get_state()));
    if (result.status == SketchProofStatus::SUCCESS && (!result.deadend_states.empty() || !result.open_states.empty() || !result.cycle.empty()))
        result.status = SketchProofStatus::FAILURE;
    if (goal && result.is_successful())
        result.plan = extract_total_ordered_plan(*goal, nodes, initial_node, task_context);
    return result;
}

}  // namespace detail

template<tyr::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context_owner, SketchView sketch, const SketchSearchOptions<Kind>& options) -> SketchProofResults<Kind>
{
    // Retain the task until the classifier's caches are destroyed, including on exceptions.
    if (options.classifier)
    {
        auto classifier = ClassifierUnsolvability<Kind>(*task_context_owner, *options.classifier);
        return detail::find_solution(task_context_owner, sketch, options, classifier);
    }
    auto classifier = NoUnsolvability {};
    return detail::find_solution(task_context_owner, sketch, options, classifier);
}

}  // namespace runir::kr::ps::base

#endif
