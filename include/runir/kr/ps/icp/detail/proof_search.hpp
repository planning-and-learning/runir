#ifndef RUNIR_KR_PS_ICP_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/icp/detail/proof_graph.hpp"
#include <urd/repr/maps/std_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include "runir/kr/ps/icp/detail/search_node.hpp"
#include "runir/kr/ps/detail/search_space.hpp"
#include "runir/kr/ps/icp/program_executor.hpp"
#include "runir/kr/ps/icp/successor_expander.hpp"
#include "runir/kr/ps/unsolvability.hpp"

#include <cstddef>
#include <optional>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <unordered_map>
#include <vector>
#include <yggdrasil/core/chrono.hpp>

namespace runir::kr::ps::icp
{

namespace detail
{

template<tyr::TaskKind Kind, UnsolvabilityConcept<Kind> Unsolvability>
auto search(const TaskContextPtr<Kind>& task, ProgramView program, const ProgramSearchOptions<Kind>& options, Unsolvability& classifier)
    -> ProgramProofResults<Kind>
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
    urd::Store<urd::sequence<SearchNode<Kind>>> nodes;
    urd::Store<urd::sequence<Predecessor>> predecessors;
    urd::Store<urd::map<ygg::uint_t, std::size_t>, urd::stable_refs> positions;
    urd::Store<urd::sequence<std::size_t>> pending;
    std::optional<std::size_t> goal;
    const auto admit = [&](ProgramStateView<Kind> state,
                           std::optional<std::size_t> parent,
                           std::optional<tyr::formalism::planning::ActionBindingView> action) -> std::optional<std::size_t>
    {
        const auto found = positions.find(ygg::uint_t(state.index().value));
        if (found != positions.end())
            return found->second;
        if (nodes.size() >= options.max_num_states)
            return std::nullopt;
        const auto index = nodes.size();
        const auto is_goal = static_goal && goal_strategy.is_dynamic_goal_satisfied(initial.get_state(), state.get_state());
        const auto unsolvable = !is_goal && classifier.is_unsolvable(state.get_state());
        nodes.push_back({ state, parent, action, is_goal, unsolvable });
        positions.emplace(ygg::uint_t(state.index().value), index);
        pending.push_back(index);
        return index;
    };
    if (!admit(initial, std::nullopt, std::nullopt))
        result.status = ProofStatus::OUT_OF_STATES;
    while (!pending.empty() && result.status == ProofStatus::SUCCESS)
    {
        if (stop())
        {
            result.status = ProofStatus::OUT_OF_TIME;
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
            [&](ProgramStep<IcpFamilyTag, Kind> step)
            {
                if (stop())
                {
                    result.status = ProofStatus::OUT_OF_TIME;
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
                    result.status = ProofStatus::OUT_OF_STATES;
                    return false;
                }
                predecessors.push_back({ index, *target, action, step.rule });
                return options.universal;
            },
            stop,
            options.universal);
        if (result.status == ProofStatus::SUCCESS && stop())
            result.status = ProofStatus::OUT_OF_TIME;
    }
    build_proof_graph(result, nodes, predecessors);
    if (result.status == ProofStatus::SUCCESS && (!result.deadend_states.empty() || !result.open_states.empty() || !result.cycle.empty()))
        result.status = ProofStatus::FAILURE;
    if (goal && result.is_successful())
        result.plan = runir::kr::ps::detail::extract_total_ordered_plan<Kind>(
            nodes[*goal],
            [&](const auto& node) { return node.parent ? &nodes[*node.parent] : nullptr; },
            initial_node,
            *task);
    return result;
}

}  // namespace detail

template<tyr::TaskKind Kind>
auto find_solution(const TaskContextPtr<Kind>& task, ProgramView program, const ProgramSearchOptions<Kind>& options) -> ProgramProofResults<Kind>
{
    if (!task)
        throw std::invalid_argument("ICP find_solution requires a task context.");
    if (options.classifier)
    {
        auto classifier = ClassifierUnsolvability<Kind>(*task, *options.classifier);
        return detail::search(task, program, options, classifier);
    }

    auto classifier = NoUnsolvability<Kind> {};
    return detail::search(task, program, options, classifier);
}

}  // namespace runir::kr::ps::icp

#endif
