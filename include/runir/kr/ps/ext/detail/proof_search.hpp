#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/proof_builder.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <chrono>
#include <deque>
#include <random>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/portable_shuffle.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    using Step = detail::ModuleProgramStep<Kind>;
    using Outcome = detail::ModuleProgramOutcome;
    using Status = ModuleProgramProofStatus;
    struct Obligation
    {
        std::vector<std::size_t> alternatives;
        bool choice;
    };
    struct Expansion
    {
        ExecutionStateView<Kind> state;
        std::vector<Step> steps;
        std::vector<Obligation> obligations;
        std::vector<std::optional<graphs::VertexIndex>> targets;
        bool expanded = false;
        bool visited = false;
        bool reported_deadend = false;
        bool reported_open = false;

        explicit Expansion(ExecutionStateView<Kind> state_) : state(std::move(state_)) {}
    };
    struct Work
    {
        graphs::VertexIndex vertex;
        std::optional<std::size_t> choice = std::nullopt;
    };
    struct ChoiceFrame
    {
        Work work;
        std::size_t alternative;
        std::vector<Work> open;
        std::size_t visited_size;
        std::size_t edge_count;
        std::size_t plan_length;
        bool failed;
    };

    const auto& search_context = *task_context->search_context;
    const auto initial_node = search_context.successor_generator->get_initial_node(*search_context.state_repository, *search_context.axiom_evaluator);
    auto proof = detail::ModuleProgramProofBuilder<Kind>(std::move(task_context), program, options.classifier);
    auto expansions = std::deque<Expansion> {};
    auto open = std::vector<Work> {};
    auto choices = std::vector<ChoiceFrame> {};
    auto visited = graphs::VertexIndexList {};
    auto selected_edges = std::vector<std::tuple<graphs::VertexIndex, graphs::VertexIndex, ygg::uint_t>> {};
    auto plan_steps = tyr::planning::LabeledNodeList<Kind> {};
    auto failed = false;
    auto num_choice_points = ygg::uint_t(0);
    auto num_binding_attempts = ygg::uint_t(0);
    auto num_backtracks = ygg::uint_t(0);
    auto random = std::mt19937_64(options.random_seed);
    const auto started_at = std::chrono::steady_clock::now();
    const auto out_of_time = [&]() { return options.max_time && std::chrono::steady_clock::now() - started_at >= *options.max_time; };
    const auto finish = [&](Status status, std::optional<ygg::uint_t> choice_depth = std::nullopt)
    {
        auto result = proof.finish(status);
        result.choice_depth = choice_depth;
        result.num_choice_points = num_choice_points;
        result.num_binding_attempts = num_binding_attempts;
        result.num_backtracks = num_backtracks;
        return result;
    };
    const auto vertex_for = [&](ExecutionStateView<Kind> state, bool initial = false, bool alive = true, bool unsolvable = false)
        -> std::optional<graphs::VertexIndex>
    {
        const auto result = proof.get_or_create_vertex(state, initial, alive, unsolvable, options.max_num_states);
        if (!result)
            return std::nullopt;
        if (result->second)
            expansions.emplace_back(std::move(state));
        return result->first;
    };
    const auto enqueue = [&](graphs::VertexIndex vertex)
    {
        if (!expansions[vertex].visited)
        {
            expansions[vertex].visited = true;
            visited.push_back(vertex);
            open.push_back({ vertex });
        }
    };
    const auto apply_step = [&](graphs::VertexIndex source, std::size_t step_index, const Obligation* choice = nullptr) -> std::optional<Status>
    {
        auto& expansion = expansions[source];
        const auto& step = expansion.steps[step_index];
        if (step.status == Outcome::OUT_OF_TIME)
            return Status::OUT_OF_TIME;
        if (step.status == Outcome::OUT_OF_STATES)
            return Status::OUT_OF_STATES;

        const auto applied = step.status == Outcome::APPLIED || step.status == Outcome::RESTORED_CALLER;
        if (choice && applied)
            ++num_binding_attempts;
        const auto empty_choice = !applied && choice;
        if (applied || (step.get_target().get_phase() == ExecutionPhase::INTERNAL && !empty_choice))
        {
            auto& target = expansion.targets[step_index];
            if (!target)
            {
                target = vertex_for(step.get_target(), false, applied, !applied);
                if (!target)
                    return Status::OUT_OF_STATES;
                proof.add_edge(source, *target, step.get_state_transition(), step.rule);
            }
            selected_edges.emplace_back(source, *target, choice && choice->alternatives.size() > 1);
            if (applied)
            {
                if (!options.universal)
                    plan_steps.insert(plan_steps.end(), step.plan_suffix.begin(), step.plan_suffix.end());
                enqueue(*target);
            }
            else
            {
                if (!expansions[*target].reported_deadend)
                    proof.add_deadend_state(*target);
                expansions[*target].reported_deadend = true;
            }
        }
        else if (empty_choice)
        {
            // Failure belongs to this choose obligation, not to the shared source state.
            if (!expansion.reported_deadend)
                proof.add_deadend_state(source);
            expansion.reported_deadend = true;
        }
        else
        {
            if (!expansion.reported_open)
                proof.add_open_state(source);
            expansion.reported_open = true;
        }
        failed |= !applied;
        return std::nullopt;
    };
    const auto choice_depth = [&](graphs::VertexIndex initial) -> std::optional<ygg::uint_t>
    {
        // Only the current attempt determines success; the proof graph also retains abandoned bindings.
        auto graph = graphs::StaticGraphBuilder<std::tuple<>, ygg::uint_t> {};
        for (std::size_t i = 0; i < expansions.size(); ++i)
            graph.add_vertex();
        for (const auto& [source, target, weight] : selected_edges)
            graph.add_directed_edge(source, target, weight);
        if (!graphs::find_cycle(graph).empty())
            return std::nullopt;

        // The topological order is sinks first. Shared continuations must retain the longest incoming path.
        auto depths = std::vector<ygg::uint_t>(expansions.size(), 0);
        for (const auto source : graphs::algorithms::topological_sort(graph))
            for (const auto edge : graph.get_out_edge_indices(source))
                depths[source] = std::max(depths[source], graph.get_edge(edge).get_property() + depths[graph.get_target(edge)]);
        return depths[initial];
    };

    const auto initial_vertex = vertex_for(proof.initial_state(), true);
    if (!initial_vertex)
        return finish(Status::OUT_OF_STATES);
    enqueue(*initial_vertex);

    while (true)
    {
        if (out_of_time())
            return finish(Status::OUT_OF_TIME);

        if (open.empty())
        {
            if (!failed)
                if (const auto depth = choice_depth(*initial_vertex))
                    return finish(Status::SUCCESS, depth);

            auto resumed = false;
            while (!choices.empty())
            {
                auto& frame = choices.back();
                const auto& expansion = expansions[frame.work.vertex];
                const auto& obligation = expansion.obligations[*frame.work.choice];
                const auto& alternatives = obligation.alternatives;
                if (expansion.steps[alternatives[frame.alternative]].status == Outcome::APPLIED)
                    ++num_backtracks;
                open = frame.open;
                while (visited.size() > frame.visited_size)
                {
                    expansions[visited.back()].visited = false;
                    visited.pop_back();
                }
                selected_edges.resize(frame.edge_count);
                plan_steps.erase(plan_steps.begin() + frame.plan_length, plan_steps.end());
                failed = frame.failed;

                if (++frame.alternative < alternatives.size())
                {
                    if (const auto limit = apply_step(frame.work.vertex, alternatives[frame.alternative], &obligation))
                        return finish(*limit);
                    resumed = true;
                    break;
                }
                choices.pop_back();
            }
            if (!resumed)
                return finish(Status::FAILURE);
            continue;
        }

        const auto work = open.back();
        open.pop_back();
        auto& expansion = expansions[work.vertex];
        if (work.choice)
        {
            const auto& obligation = expansion.obligations[*work.choice];
            num_choice_points += obligation.alternatives.size() > 1;
            // ponytail: copy the pending frontier only at choose; use a persistent worklist if these snapshots dominate memory.
            choices.push_back({ work, 0, open, visited.size(), selected_edges.size(), plan_steps.size(), failed });
            if (const auto limit = apply_step(work.vertex, obligation.alternatives.front(), &obligation))
                return finish(*limit);
            continue;
        }

        if (proof.is_goal(expansion.state.get_state()))
        {
            if (!options.universal)
            {
                proof.set_plan(tyr::planning::Plan<Kind>(initial_node, std::move(plan_steps)));
                return finish(Status::SUCCESS, choice_depth(*initial_vertex));
            }
            continue;
        }
        if (proof.is_unsolvable(work.vertex))
        {
            if (!expansion.reported_deadend)
                proof.add_deadend_state(work.vertex);
            expansion.reported_deadend = true;
            failed = true;
            continue;
        }

        if (!expansion.expanded)
        {
            if (options.universal || options.shuffle_choice_points)
                proof.template steps<EagerExpansionPolicy>(expansion.state, out_of_time, expansion.steps);
            else
                proof.template steps<LazyExpansionPolicy>(expansion.state, out_of_time, expansion.steps);
            if (out_of_time())
                return finish(Status::OUT_OF_TIME);
            if (options.shuffle_choice_points)
                ygg::portable_shuffle(expansion.steps.begin(), expansion.steps.end(), random);

            for (std::size_t i = 0; i < expansion.steps.size(); ++i)
            {
                const auto& step = expansion.steps[i];
                if (!step.rule)
                {
                    if (options.universal || i == 0)
                        expansion.obligations.push_back({ { i }, false });
                    continue;
                }
                ygg::visit(
                    [&](auto rule)
                    {
                        if constexpr (ChooseRuleView<decltype(rule)>)
                        {
                            if (!options.universal && i != 0
                                && (!expansion.steps.front().rule || step.rule->get_index() != expansion.steps.front().rule->get_index()))
                                return;
                            const auto group = std::find_if(expansion.obligations.begin(), expansion.obligations.end(), [&](const auto& obligation)
                            {
                                const auto& first = expansion.steps[obligation.alternatives.front()];
                                return obligation.choice && first.rule->get_index() == step.rule->get_index();
                            });
                            if (group == expansion.obligations.end())
                                expansion.obligations.push_back({ { i }, true });
                            else
                                group->alternatives.push_back(i);
                        }
                        else if (options.universal || i == 0)
                            expansion.obligations.push_back({ { i }, false });
                    },
                    step.rule->get_variant());
            }
            expansion.targets.resize(expansion.steps.size());
            expansion.expanded = true;
        }

        for (std::size_t i = 0; i < expansion.obligations.size(); ++i)
        {
            if (out_of_time())
                return finish(Status::OUT_OF_TIME);
            const auto& obligation = expansion.obligations[i];
            if (obligation.choice)
                open.push_back({ work.vertex, i });
            else if (const auto limit = apply_step(work.vertex, obligation.alternatives.front()))
                return finish(*limit);
        }
    }
}

}  // namespace runir::kr::ps::ext

#endif
