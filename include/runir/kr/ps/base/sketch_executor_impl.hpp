#ifndef RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_IMPL_HPP_
#define RUNIR_KR_PS_BASE_SKETCH_EXECUTOR_IMPL_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/ps/base/sketch_executor_data.hpp"
#include "runir/kr/ps/base/successor_expander.hpp"

#include <chrono>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <tyr/planning/algorithms/portable_shuffle.hpp>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::base::detail
{

template<tyr::planning::TaskKind Kind>
auto execute_direct(runir::kr::TaskContextPtr<Kind> task_context_owner, SketchView sketch, const SketchSearchOptions<Kind>& options) -> SketchProofResults<Kind>
{
    auto& task_context = *task_context_owner;
    const auto& search_context = *task_context.search_context;
    auto result = SketchProofResults<Kind> {};
    result.task_context_owner = task_context_owner;
    auto builder = SketchProofGraphBuilder<Kind> {};
    auto state_to_vertex = ygg::UnorderedMap<tyr::planning::StateView<Kind>, graphs::VertexIndex> {};
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*search_context.task);
    auto expander = SuccessorExpander<Kind>(task_context, sketch);
    const auto initial_node = search_context.successor_generator->get_initial_node();
    const auto initial_state = initial_node.get_state();
    const auto static_goal_satisfied = goal_strategy.is_static_goal_satisfied(*search_context.task);
    const auto started_at = std::chrono::steady_clock::now();
    auto random = std::mt19937_64(options.random_seed);

    const auto out_of_time = [&]() { return options.max_time && std::chrono::steady_clock::now() - started_at >= *options.max_time; };

    const auto is_goal = [&](const tyr::planning::StateView<Kind>& state)
    { return static_goal_satisfied && goal_strategy.is_dynamic_goal_satisfied(initial_state, state); };

    const auto make_label = [&](const tyr::planning::StateView<Kind>& state)
    {
        const auto goal = is_goal(state);
        return datasets::AnnotatedStateGraphVertexLabel<Kind> {
            state, goal ? ygg::float_t(0) : std::numeric_limits<ygg::float_t>::infinity(), state.get_index() == initial_state.get_index(), goal, true, false
        };
    };

    const auto get_or_create_vertex = [&](const tyr::planning::StateView<Kind>& state) -> std::optional<std::pair<graphs::VertexIndex, bool>>
    {
        if (const auto it = state_to_vertex.find(state); it != state_to_vertex.end())
            return std::pair(it->second, false);
        if (state_to_vertex.size() >= options.max_num_states)
            return std::nullopt;

        const auto vertex = builder.add_vertex(make_label(state));
        state_to_vertex.emplace(state, vertex);
        return std::pair(vertex, true);
    };

    auto finish = [&](SketchProofStatus status)
    {
        result.status = status;
        auto graph = std::make_shared<SketchProofGraph<Kind>>(std::move(builder));
        result.cycle = graphs::find_cycle(*graph);
        if (result.status == SketchProofStatus::SUCCESS && !result.cycle.empty())
            result.status = SketchProofStatus::FAILURE;
        result.graph = std::move(graph);
        return std::move(result);
    };

    const auto initial = get_or_create_vertex(initial_state);
    if (!initial)
        return finish(SketchProofStatus::OUT_OF_STATES);
    const auto [initial_vertex, initial_created] = *initial;
    static_cast<void>(initial_created);

    auto open = graphs::VertexIndexList { initial_vertex };
    auto explored = ygg::UnorderedSet<graphs::VertexIndex> {};

    while (!open.empty())
    {
        if (out_of_time())
            return finish(SketchProofStatus::OUT_OF_TIME);

        const auto source = open.back();
        open.pop_back();
        if (!explored.insert(source).second)
            continue;

        const auto& source_state = builder.get_vertex(source).get_property().state;
        if (is_goal(source_state))
            continue;

        auto context = expander.context_at(source_state);
        auto successors = expander.labeled_successors(context);
        if (out_of_time())
            return finish(SketchProofStatus::OUT_OF_TIME);
        if (options.shuffle_labeled_succ_nodes)
            tyr::planning::portable_shuffle(successors.begin(), successors.end(), random);

        auto accepted = expander.accepted_successors(context, successors, out_of_time);
        if (out_of_time())
            return finish(SketchProofStatus::OUT_OF_TIME);
        if (accepted.empty())
        {
            result.open_states.push_back(source);
            continue;
        }
        if (!options.universal)
            accepted.erase(accepted.begin() + 1, accepted.end());

        for (const auto& [successor, rule] : accepted)
        {
            if (out_of_time())
                return finish(SketchProofStatus::OUT_OF_TIME);
            const auto target_result = get_or_create_vertex(successor.node.get_state());
            if (!target_result)
                return finish(SketchProofStatus::OUT_OF_STATES);
            const auto [target, created] = *target_result;
            builder.add_directed_edge(source, target, SketchProofEdgeLabel { datasets::StateGraphEdgeLabel { successor.label, ygg::float_t(1) }, rule });
            if (created || !explored.contains(target))
                open.push_back(target);
        }
    }

    return finish(result.open_states.empty() ? SketchProofStatus::SUCCESS : SketchProofStatus::FAILURE);
}

}  // namespace runir::kr::ps::base::detail

namespace runir::kr::ps::base
{

template<tyr::planning::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context_owner, SketchView sketch, const SketchSearchOptions<Kind>& options) -> SketchProofResults<Kind>
{
    return detail::execute_direct(std::move(task_context_owner), sketch, options);
}

}  // namespace runir::kr::ps::base

#endif
