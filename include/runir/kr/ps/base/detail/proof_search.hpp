#ifndef RUNIR_KR_PS_BASE_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/base/binding_order.hpp"
#include "runir/kr/ps/base/detail/proof_graph.hpp"
#include "runir/kr/ps/base/detail/search_space.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/sketch_executor.hpp"
#include "runir/kr/ps/base/successor_expander.hpp"
#include "runir/kr/ps/unsolvability.hpp"
#include "runir/kr/task_context.hpp"

#include <limits>
#include <optional>
#include <random>
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

template<tyr::TaskKind Kind, typename BindingOrder, typename Unsolvability>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context_owner,
                   SketchView sketch,
                   const SketchSearchOptions<Kind>& options,
                   BindingOrder order,
                   Unsolvability& classifier) -> SketchProofResults<Kind>
{
    constexpr auto unreached = SearchNode<Kind>::unreached;
    auto& task_context = *task_context_owner;
    const auto& search_context = *task_context.search_context;
    auto result = SketchProofResults<Kind> {};
    result.task_context_owner = task_context_owner;
    auto nodes = ygg::SegmentedVector<SearchNode<Kind>> {};
    auto predecessors = std::vector<detail::Predecessor<Kind>> {};
    auto num_reached = std::size_t(0);
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*search_context.task);
    auto expander = SuccessorExpander<Kind>(task_context, sketch);
    const auto initial_node = search_context.successor_generator->get_initial_node(*search_context.state_repository, *search_context.axiom_evaluator);
    const auto initial_state = initial_node.get_state();
    const auto static_goal_satisfied = goal_strategy.is_static_goal_satisfied(*search_context.task);
    const auto stopwatch = options.max_time ? std::optional<ygg::CountdownWatch>(*options.max_time) : std::nullopt;

    const auto out_of_time = [&]() { return stopwatch && stopwatch->has_finished(); };

    const auto is_goal = [&](const tyr::planning::StateView<Kind>& state)
    { return static_goal_satisfied && goal_strategy.is_dynamic_goal_satisfied(initial_state, state); };

    const auto get_or_create_state = [&](const tyr::planning::StateView<Kind>& state) -> std::optional<std::pair<ygg::Index<tyr::planning::State<Kind>>, bool>>
    {
        const auto index = state.get_index();
        const auto position = std::size_t(ygg::uint_t(index));
        if (position < nodes.size() && nodes[position].step != unreached)
            return std::pair(index, false);
        if (num_reached >= options.max_num_states)
            return std::nullopt;
        auto& node = detail::get_or_create_search_node(index, nodes);
        node.step = num_reached++;
        node.is_goal = is_goal(state);
        node.is_unsolvable = !node.is_goal && classifier.is_unsolvable(state);
        return std::pair(index, true);
    };

    auto finish = [&](SketchProofStatus status, std::optional<tyr::planning::PackedNode<Kind>> goal = std::nullopt)
    {
        result.status = status;
        detail::build_proof_graph<Kind>(result, nodes, predecessors, initial_state.get_index());
        if (result.status == SketchProofStatus::SUCCESS && (!result.deadend_states.empty() || !result.open_states.empty() || !result.cycle.empty()))
            result.status = SketchProofStatus::FAILURE;
        if (goal && result.is_successful())
            result.plan = detail::extract_total_ordered_plan(std::move(*goal), nodes);
        return std::move(result);
    };

    const auto initial = get_or_create_state(initial_state);
    if (!initial)
        return finish(SketchProofStatus::OUT_OF_STATES);
    auto open = tyr::planning::PackedNodeList<Kind> { initial_node.pack() };

    while (!open.empty())
    {
        if (out_of_time())
            return finish(SketchProofStatus::OUT_OF_TIME);

        const auto packed_node = std::move(open.back());
        open.pop_back();
        const auto source = packed_node.get_state().get_index();
        auto& source_node = detail::get_or_create_search_node(source, nodes);
        if (source_node.is_goal)
        {
            if (!options.universal)
                return finish(SketchProofStatus::SUCCESS, packed_node);
            continue;
        }
        if (source_node.is_unsolvable)
        {
            source_node.is_deadend = true;
            continue;
        }

        ++result.statistics.num_expanded;
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
            const auto [target, created] = *target_result;
            predecessors.push_back({ source, target, successor.label, ygg::float_t(1), rule });
            if (created)
            {
                auto& target_node = detail::get_or_create_search_node(target, nodes);
                target_node.parent_node = packed_node;
                target_node.action = successor.label;
                open.push_back(successor.node.pack());
            }
            is_open = false;
            return options.universal;
        };

        expander.for_each_successor(node, result.statistics, order, accept_successor, out_of_time);
        if (expansion_status != SketchProofStatus::SUCCESS)
            return finish(expansion_status);
        if (out_of_time())
            return finish(SketchProofStatus::OUT_OF_TIME);
        source_node.is_open = is_open;
    }

    return finish(SketchProofStatus::SUCCESS);
}

}  // namespace detail

template<tyr::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context_owner, SketchView sketch, const SketchSearchOptions<Kind>& options) -> SketchProofResults<Kind>
{
    const auto search = [&](auto& classifier)
    {
        // Retain the task until the classifier's caches are destroyed, including on exceptions.
        if (options.shuffle_choice_points)
        {
            auto random = std::mt19937_64(options.random_seed);
            return detail::find_solution(task_context_owner, sketch, options, Shuffled(random), classifier);
        }
        return detail::find_solution(task_context_owner, sketch, options, InOrder {}, classifier);
    };

    if (options.classifier)
    {
        auto classifier = ClassifierUnsolvability<Kind>(*task_context_owner, *options.classifier);
        return search(classifier);
    }
    auto classifier = NoUnsolvability {};
    return search(classifier);
}

}  // namespace runir::kr::ps::base

#endif
