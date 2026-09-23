#ifndef RUNIR_KR_PS_BASE_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_BASE_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/dl/semantics/uns/state_evaluation_context.hpp"
#include "runir/kr/ps/base/detail/proof_graph.hpp"
#include "runir/kr/ps/base/detail/search_space.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/sketch_executor.hpp"
#include "runir/kr/ps/base/successor_expander.hpp"
#include "runir/kr/task_context.hpp"
#include "runir/kr/uns/classify.hpp"

#include <limits>
#include <optional>
#include <random>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/containers/segmented_vector.hpp>
#include <yggdrasil/containers/unordered_set.hpp>
#include <yggdrasil/core/chrono.hpp>
#include <yggdrasil/core/portable_shuffle.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::base
{

template<tyr::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context_owner, SketchView sketch, const SketchSearchOptions<Kind>& options) -> SketchProofResults<Kind>
{
    using StateIndex = detail::StateIndex<Kind>;
    using SearchNode = detail::SearchNode<Kind>;
    constexpr auto unreached = SearchNode::unreached;
    auto& task_context = *task_context_owner;
    const auto& search_context = *task_context.search_context;
    auto result = SketchProofResults<Kind> {};
    result.task_context_owner = task_context_owner;
    auto nodes = ygg::SegmentedVector<SearchNode> {};
    auto predecessors = ygg::UnorderedSet<detail::Predecessor<Kind>> {};
    auto deadends = std::vector<StateIndex> {};
    auto open_states = std::vector<StateIndex> {};
    auto num_reached = std::size_t(0);
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*search_context.task);
    auto expander = SuccessorExpander<Kind>(task_context, sketch);
    auto classifier_caches = runir::kr::dl::semantics::DenotationCaches<runir::kr::UnsFamilyTag> {};
    const auto initial_node = search_context.successor_generator->get_initial_node(*search_context.state_repository, *search_context.axiom_evaluator);
    const auto initial_state = initial_node.get_state();
    const auto static_goal_satisfied = goal_strategy.is_static_goal_satisfied(*search_context.task);
    const auto stopwatch = options.max_time ? std::optional<ygg::CountdownWatch>(*options.max_time) : std::nullopt;
    auto random = std::mt19937_64(options.random_seed);

    const auto out_of_time = [&]() { return stopwatch && stopwatch->has_finished(); };

    const auto is_goal = [&](const tyr::planning::StateView<Kind>& state)
    { return static_goal_satisfied && goal_strategy.is_dynamic_goal_satisfied(initial_state, state); };

    const auto get_or_create_state = [&](const tyr::planning::StateView<Kind>& state) -> std::optional<std::pair<StateIndex, bool>>
    {
        const auto index = state.get_index();
        const auto position = std::size_t(ygg::uint_t(index));
        if (position < nodes.size() && nodes[position].discovery_order != unreached)
            return std::pair(index, false);
        if (num_reached >= options.max_num_states)
            return std::nullopt;
        auto& node = detail::get_or_create_search_node(index, nodes);
        node.discovery_order = num_reached++;
        node.is_goal = is_goal(state);
        if (!node.is_goal && options.classifier)
        {
            classifier_caches.clear(false);
            auto context = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, Kind>(state,
                                                                                                      task_context.dl_builder,
                                                                                                      *task_context.dl_denotation_repository,
                                                                                                      task_context.dl_builder.get_workspace(),
                                                                                                      classifier_caches);
            node.is_unsolvable = runir::kr::uns::classify(*options.classifier, context);
        }
        return std::pair(index, true);
    };

    auto finish = [&](SketchProofStatus status, std::optional<StateIndex> goal = std::nullopt)
    {
        result.status = status;
        detail::build_proof_graph<Kind>(result, nodes, predecessors, initial_state.get_index(), deadends, open_states);
        if (result.status == SketchProofStatus::SUCCESS && !result.cycle.empty())
            result.status = SketchProofStatus::FAILURE;
        if (goal && result.is_successful())
            result.plan = detail::extract_total_ordered_plan(*goal, nodes, initial_node.pack());
        return std::move(result);
    };

    const auto initial = get_or_create_state(initial_state);
    if (!initial)
        return finish(SketchProofStatus::OUT_OF_STATES);
    auto open = std::vector<StateIndex> { initial->first };
    auto successors = std::vector<tyr::planning::LabeledNode<Kind>> {};
    auto accepted = std::vector<typename SuccessorExpander<Kind>::AcceptedSuccessor> {};

    while (!open.empty())
    {
        if (out_of_time())
            return finish(SketchProofStatus::OUT_OF_TIME);

        const auto source = open.back();
        open.pop_back();
        auto& source_node = detail::get_or_create_search_node(source, nodes);
        if (source_node.visited)
            continue;
        source_node.visited = true;
        if (source_node.is_goal)
        {
            if (!options.universal)
                return finish(SketchProofStatus::SUCCESS, source);
            continue;
        }
        if (source_node.is_unsolvable)
        {
            deadends.push_back(source);
            continue;
        }

        ++result.statistics.num_expanded;
        const auto source_state = search_context.state_repository->get_registered_state(source);
        auto expansion_status = SketchProofStatus::SUCCESS;
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
            predecessors.emplace(source, target, successor.label, ygg::float_t(1), rule, predecessors.size());
            auto& target_node = detail::get_or_create_search_node(target, nodes);
            target_node.boundary = target_node.boundary || !created;
            if (created)
            {
                target_node.parent_state = source;
                target_node.action = successor.label;
                target_node.metric = successor.node.get_metric();
            }
            if (!target_node.visited)
                open.push_back(target);
            source_node.has_successor = true;
            return options.universal;
        };

        if (options.shuffle_choice_points)
        {
            auto& generator = *search_context.successor_generator;
            const auto node = generator.get_node(*search_context.state_repository, source);
            generator.get_labeled_successor_nodes(node, *search_context.state_repository, *search_context.axiom_evaluator, successors);
            result.statistics.num_generated += successors.size();
            if (out_of_time())
                return finish(SketchProofStatus::OUT_OF_TIME);
            ygg::portable_shuffle(successors.begin(), successors.end(), random);
            accepted.clear();
            expander.get_environment().get_dl_caches().clear(false);
            for (const auto& successor : successors)
            {
                if (out_of_time())
                    break;
                if (const auto rule = expander.matching_rule_until(source_state, successor.node.get_state(), out_of_time))
                    accepted.emplace_back(successor, *rule);
            }
            if (out_of_time())
                return finish(SketchProofStatus::OUT_OF_TIME);
            for (const auto& [successor, rule] : accepted)
                if (!accept_successor(successor, rule))
                    break;
        }
        else
        {
            expander.get_environment().get_dl_caches().clear(false);
            auto& generator = *search_context.successor_generator;
            const auto node = generator.get_node(*search_context.state_repository, source_state.get_index());
            generator.for_each_labeled_successor_node(node, *search_context.state_repository, *search_context.axiom_evaluator,
                [&](const auto& successor)
                {
                    ++result.statistics.num_generated;
                    const auto rule = expander.matching_rule_until(source_state, successor.node.get_state(), out_of_time);
                    if (out_of_time())
                    {
                        expansion_status = SketchProofStatus::OUT_OF_TIME;
                        return false;
                    }
                    return !rule || accept_successor(successor, *rule);
                });
        }
        if (expansion_status != SketchProofStatus::SUCCESS)
            return finish(expansion_status);
        if (out_of_time())
            return finish(SketchProofStatus::OUT_OF_TIME);
        if (!source_node.has_successor)
            open_states.push_back(source);
    }

    return finish(deadends.empty() && open_states.empty() ? SketchProofStatus::SUCCESS : SketchProofStatus::FAILURE);
}

}  // namespace runir::kr::ps::base

#endif
