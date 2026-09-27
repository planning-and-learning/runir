#ifndef RUNIR_KR_PS_ICP_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/icp/detail/proof_graph.hpp"
#include "runir/kr/ps/icp/detail/search_space.hpp"
#include "runir/kr/ps/icp/program_executor.hpp"
#include "runir/kr/ps/icp/successor_expander.hpp"
#include "runir/kr/ps/unsolvability.hpp"

#include <cstddef>
#include <optional>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <unordered_map>
#include <vector>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::ps::icp
{

namespace detail
{

template<tyr::TaskKind Kind, typename Unsolvability>
auto search(TaskContextPtr<Kind> task, ProgramView program, const ProgramSearchOptions<Kind>& options, Unsolvability& classifier) -> ProgramProofResults<Kind>
{
    auto result = ProgramProofResults<Kind> {};
    result.task_context_owner = task;
    auto& planning = *task->search_context;
    auto expander = SuccessorExpander<Kind>(task, program);
    const auto initial_node = planning.successor_generator->get_packed_initial_node(*planning.state_repository, *planning.axiom_evaluator);
    const auto initial = expander.initial_state(initial_node.get_state().unpack());
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*planning.task);
    const auto static_goal = goal_strategy.is_static_goal_satisfied(*planning.task);
    const auto watch = options.max_time ? std::optional<ygg::CountdownWatch>(*options.max_time) : std::nullopt;
    const auto stop = [&]() { return watch && watch->has_finished(); };
    std::vector<SearchNode<Kind>> nodes;
    std::vector<Predecessor> predecessors;
    std::unordered_map<ygg::uint_t, std::size_t> positions;
    std::vector<std::size_t> pending;
    std::optional<std::size_t> goal;
    const auto admit = [&](ProgramStateView<Kind> state,
                           std::optional<std::size_t> parent,
                           std::optional<tyr::formalism::planning::ActionBindingView> action) -> std::optional<std::size_t>
    {
        const auto found = positions.find(ygg::uint_t(state.get_index()));
        if (found != positions.end())
            return found->second;
        if (nodes.size() >= options.max_num_states)
            return std::nullopt;
        const auto index = nodes.size();
        const auto is_goal = static_goal && goal_strategy.is_dynamic_goal_satisfied(initial.get_state(), state.get_state());
        const auto unsolvable = !is_goal && classifier.is_unsolvable(state.get_state());
        nodes.push_back({ state, parent, action, is_goal, unsolvable });
        positions.emplace(ygg::uint_t(state.get_index()), index);
        pending.push_back(index);
        return index;
    };
    if (!admit(initial, std::nullopt, std::nullopt))
        result.status = ProgramProofStatus::OUT_OF_STATES;
    while (!pending.empty() && result.status == ProgramProofStatus::SUCCESS)
    {
        if (stop())
        {
            result.status = ProgramProofStatus::OUT_OF_TIME;
            break;
        }
        const auto index = pending.back();
        pending.pop_back();
        if (nodes[index].is_goal)
        {
            if (!options.universal)
            {
                goal = index;
                break;
            }
            continue;
        }
        if (nodes[index].is_unsolvable)
        {
            nodes[index].is_deadend = true;
            continue;
        }
        const auto state = nodes[index].state;
        ++result.statistics.num_expanded;
        expander.for_each_successor(
            state,
            result.statistics,
            [&](ProgramStep<Kind> step)
            {
                if (stop())
                {
                    result.status = ProgramProofStatus::OUT_OF_TIME;
                    return false;
                }
                if (step.status != ProgramOutcome::APPLIED)
                {
                    if (step.status == ProgramOutcome::NO_APPLICABLE_ACTION)
                        nodes[index].is_open = true;
                    else
                        nodes[index].is_deadend = true;
                    return options.universal;
                }
                const auto action = step.state_transition ? std::optional(step.state_transition->action) : std::nullopt;
                const auto target = admit(step.target, index, action);
                if (!target)
                {
                    result.status = ProgramProofStatus::OUT_OF_STATES;
                    return false;
                }
                predecessors.push_back({ index, *target, action, step.rule });
                return options.universal;
            },
            stop,
            options.universal);
        if (result.status == ProgramProofStatus::SUCCESS && stop())
            result.status = ProgramProofStatus::OUT_OF_TIME;
    }
    build_proof_graph(result, nodes, predecessors);
    if (result.status == ProgramProofStatus::SUCCESS && (!result.deadend_states.empty() || !result.open_states.empty() || !result.cycle.empty()))
        result.status = ProgramProofStatus::FAILURE;
    if (goal && result.is_successful())
        result.plan = extract_total_ordered_plan(*goal, nodes, initial_node, *task);
    return result;
}

}  // namespace detail

template<tyr::TaskKind Kind>
auto find_solution(TaskContextPtr<Kind> task, ProgramView program, const ProgramSearchOptions<Kind>& options) -> ProgramProofResults<Kind>
{
    if (!task)
        throw std::invalid_argument("ICP find_solution requires a task context.");
    if (options.classifier)
    {
        auto classifier = ClassifierUnsolvability<Kind>(*task, *options.classifier);
        return detail::search(task, program, options, classifier);
    }

    auto classifier = NoUnsolvability {};
    return detail::search(task, program, options, classifier);
}

}  // namespace runir::kr::ps::icp

#endif
