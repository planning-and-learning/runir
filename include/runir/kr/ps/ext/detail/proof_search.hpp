#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/graphs/cycle.hpp"
#include "runir/kr/dl/semantics/uns/state_evaluation_context.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/program_executor.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"
#include "runir/kr/task_context.hpp"
#include "runir/kr/uns/classify.hpp"

#include <algorithm>
#include <chrono>
#include <deque>
#include <memory>
#include <optional>
#include <random>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/core/portable_shuffle.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context,
                   ProgramView program,
                   const ProgramSearchOptions<Kind>& options) -> ProgramProofResults<Kind>
{
    using Step = detail::ProgramStep<Kind>;
    using Outcome = detail::ProgramOutcome;
    using Status = ProgramProofStatus;
    struct Obligation
    {
        std::vector<std::size_t> alternatives;
        bool choice;
    };
    struct Expansion
    {
        ProgramStateView<Kind> state;
        std::vector<Step> steps;
        std::vector<Obligation> obligations;
        std::vector<std::optional<graphs::VertexIndex>> targets;
        bool expanded = false;
        bool visited = false;
        bool reported_deadend = false;
        bool reported_open = false;

        explicit Expansion(ProgramStateView<Kind> state_) : state(std::move(state_)) {}
    };
    struct Work
    {
        graphs::VertexIndex vertex;
        std::optional<std::size_t> choice = std::nullopt;
        ygg::uint_t depth = 0;
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
    const auto initial_node = search_context.successor_generator->get_packed_initial_node(*search_context.state_repository, *search_context.axiom_evaluator);
    auto result = ProgramProofResults<Kind> {};
    result.task_context_owner = task_context;
    auto builder = ProgramProofGraphBuilder<Kind> {};
    auto state_to_vertex = ygg::UnorderedMap<ygg::Index<ProgramState<Kind>>, graphs::VertexIndex> {};
    auto expander = SuccessorExpander<Kind>(task_context, program);
    auto classifier_caches = runir::kr::dl::semantics::DenotationCaches<runir::kr::UnsFamilyTag> {};
    auto expansions = std::deque<Expansion> {};
    auto open = std::vector<Work> {};
    auto choices = std::vector<ChoiceFrame> {};
    auto visited = graphs::VertexIndexList {};
    auto selected_edges = std::vector<std::tuple<graphs::VertexIndex, graphs::VertexIndex, ygg::uint_t>> {};
    auto plan_steps = tyr::planning::PackedLabeledNodeList<Kind> {};
    auto failed = false;
    auto statistics = ProgramSearchStatistics {};
    auto random = std::mt19937_64(options.random_seed);
    const auto started_at = std::chrono::steady_clock::now();
    const auto out_of_time = [&]() { return options.max_time && std::chrono::steady_clock::now() - started_at >= *options.max_time; };
    const auto assess_choice_depth = [&]() -> std::optional<ygg::uint_t>
    {
        if (expansions.empty())
            return 0;

        // Only the current attempt determines success; the proof graph also retains abandoned bindings.
        auto graph = graphs::StaticGraphBuilder<std::tuple<>, ygg::uint_t> {};
        for (std::size_t i = 0; i < expansions.size(); ++i)
            graph.add_vertex();
        for (const auto& [source, target, weight] : selected_edges)
            graph.add_directed_edge(source, target, weight);
        // Cyclic attempts retain the observed traversal peak without repeatedly following their cycles.
        if (!graphs::find_cycle(graph).empty())
            return std::nullopt;

        // The topological order is sinks first. Shared continuations must retain the longest incoming path.
        auto depths = std::vector<ygg::uint_t>(expansions.size(), 0);
        for (const auto source : graphs::algorithms::topological_sort(graph))
            for (const auto edge : graph.get_out_edge_indices(source))
                depths[source] = std::max(depths[source], graph.get_edge(edge).get_property() + depths[graph.get_target(edge)]);
        statistics.max_choice_depth = std::max(statistics.max_choice_depth, depths.front());
        return depths.front();
    };

    const auto finish = [&](Status status, ygg::uint_t choice_depth = 0)
    {
        if (status != Status::SUCCESS)
            assess_choice_depth();
        result.status = status;
        auto graph = std::make_shared<ProgramProofGraph<Kind>>(std::move(builder));
        result.cycle = graphs::find_cycle(*graph);
        result.graph = std::move(graph);
        result.statistics = statistics;
        result.statistics.choice_depth = choice_depth;
        return std::move(result);
    };
    const auto vertex_for = [&](ProgramStateView<Kind> state, bool initial = false) -> std::optional<graphs::VertexIndex>
    {
        if (const auto it = state_to_vertex.find(state.get_index()); it != state_to_vertex.end())
            return it->second;
        if (state_to_vertex.size() >= options.max_num_states)
            return std::nullopt;

        const auto goal = expander.is_goal(state.get_state());
        auto unsolvable = false;
        if (!goal && options.classifier)
        {
            classifier_caches.clear(false);
            auto context = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, Kind>(state.get_state(),
                                                                                                      task_context->dl_builder,
                                                                                                      *task_context->dl_denotation_repository,
                                                                                                      task_context->dl_builder.get_workspace(),
                                                                                                      classifier_caches);
            unsolvable = runir::kr::uns::classify(*options.classifier, context);
        }
        const auto vertex = builder.add_vertex(ProgramProofVertexLabel<Kind> { state, initial, goal, !unsolvable, unsolvable });
        state_to_vertex.emplace(state.get_index(), vertex);
        expansions.emplace_back(std::move(state));
        return vertex;
    };
    const auto enqueue = [&](graphs::VertexIndex vertex, ygg::uint_t depth = 0)
    {
        if (!expansions[vertex].visited)
        {
            expansions[vertex].visited = true;
            visited.push_back(vertex);
            open.push_back({ vertex, std::nullopt, depth });
        }
    };
    const auto apply_step = [&](const Work& work, std::size_t step_index, const Obligation* choice = nullptr) -> std::optional<Status>
    {
        const auto source = work.vertex;
        auto& expansion = expansions[source];
        const auto& step = expansion.steps[step_index];
        if (step.status == Outcome::OUT_OF_TIME)
            return Status::OUT_OF_TIME;
        if (step.status == Outcome::OUT_OF_STATES)
            return Status::OUT_OF_STATES;

        const auto applied = step.status == Outcome::APPLIED || step.status == Outcome::RESTORED_CALLER;
        if (choice && applied)
            ++statistics.num_binding_attempts;
        const auto depth = work.depth + ygg::uint_t(choice && choice->alternatives.size() > 1);
        if (applied)
            statistics.max_choice_depth = std::max(statistics.max_choice_depth, depth);
        const auto empty_choice = !applied && choice;
        if (applied)
        {
            auto& target = expansion.targets[step_index];
            if (!target)
            {
                target = vertex_for(step.get_target());
                if (!target)
                    return Status::OUT_OF_STATES;
                auto label = ProgramProofEdgeLabel {};
                if (const auto transition = step.get_state_transition())
                    label.state_transition = ProgramProofStateTransition { transition->action, transition->cost };
                if (step.rule)
                    label.rule = *step.rule;
                builder.add_directed_edge(source, *target, std::move(label));
            }
            selected_edges.emplace_back(source, *target, choice && choice->alternatives.size() > 1);
            if (!options.universal)
                plan_steps.insert(plan_steps.end(), step.plan_suffix.begin(), step.plan_suffix.end());
            enqueue(*target, depth);
        }
        else if (empty_choice)
        {
            // Failure belongs to this choose obligation, not to the shared source state.
            if (!expansion.reported_deadend)
                result.deadend_states.push_back(source);
            expansion.reported_deadend = true;
        }
        else
        {
            if (!expansion.reported_open)
                result.open_states.push_back(source);
            expansion.reported_open = true;
        }
        failed |= !applied;
        return std::nullopt;
    };

    const auto initial_vertex = vertex_for(expander.initial_state(), true);
    if (!initial_vertex)
        return finish(Status::OUT_OF_STATES);
    enqueue(*initial_vertex);

    while (true)
    {
        if (out_of_time())
            return finish(Status::OUT_OF_TIME);

        if (open.empty())
        {
            const auto depth = assess_choice_depth();
            if (!failed && depth)
                return finish(Status::SUCCESS, *depth);

            auto resumed = false;
            while (!choices.empty())
            {
                auto& frame = choices.back();
                const auto& expansion = expansions[frame.work.vertex];
                const auto& obligation = expansion.obligations[*frame.work.choice];
                const auto& alternatives = obligation.alternatives;
                if (expansion.steps[alternatives[frame.alternative]].status == Outcome::APPLIED)
                    ++statistics.num_backtracks;
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
                    if (const auto limit = apply_step(frame.work, alternatives[frame.alternative], &obligation))
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
            statistics.num_choice_points += obligation.alternatives.size() > 1;
            // ponytail: copy the pending frontier only at choose; use a persistent worklist if these snapshots dominate memory.
            choices.push_back({ work, 0, open, visited.size(), selected_edges.size(), plan_steps.size(), failed });
            if (const auto limit = apply_step(work, obligation.alternatives.front(), &obligation))
                return finish(*limit);
            continue;
        }

        if (expander.is_goal(expansion.state.get_state()))
        {
            if (!options.universal)
            {
                result.plan = tyr::planning::PackedPlan<Kind>(initial_node, std::move(plan_steps));
                return finish(Status::SUCCESS, assess_choice_depth().value());
            }
            continue;
        }
        if (builder.get_vertex(work.vertex).get_property().is_unsolvable)
        {
            if (!expansion.reported_deadend)
                result.deadend_states.push_back(work.vertex);
            expansion.reported_deadend = true;
            failed = true;
            continue;
        }

        if (!expansion.expanded)
        {
            ++statistics.num_expanded;
            if (options.universal || options.shuffle_choice_points)
                expander.template steps_until<EagerExpansionPolicy>(expansion.state, out_of_time, expansion.steps);
            else
                expander.template steps_until<LazyExpansionPolicy>(expansion.state, out_of_time, expansion.steps);
            statistics.num_generated += std::ranges::count_if(expansion.steps, [](const auto& step)
            { return step.status == Outcome::APPLIED || step.status == Outcome::RESTORED_CALLER; });
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
                open.push_back({ work.vertex, i, work.depth });
            else if (const auto limit = apply_step(work, obligation.alternatives.front()))
                return finish(*limit);
        }
    }
}

}  // namespace runir::kr::ps::ext

#endif
