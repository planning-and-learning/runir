#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/dl/semantics/uns/state_evaluation_context.hpp"
#include "runir/kr/ps/ext/detail/attempt_analysis.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/proof_graph.hpp"
#include "runir/kr/ps/ext/detail/search_space.hpp"
#include "runir/kr/ps/ext/program_executor.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"
#include "runir/kr/task_context.hpp"
#include "runir/kr/uns/classify.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <optional>
#include <random>
#include <span>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/containers/segmented_vector.hpp>
#include <yggdrasil/containers/unordered_set.hpp>
#include <yggdrasil/core/chrono.hpp>
#include <yggdrasil/core/portable_shuffle.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace runir::kr::ps::ext
{

namespace detail
{
template<tyr::TaskKind Kind>
struct Work
{
    using ChoiceVariant = std::variant<Choice<runir::kr::dl::ConceptTag>, Choice<runir::kr::dl::RoleTag>>;
    static constexpr auto no_entry = std::numeric_limits<std::size_t>::max();

    ygg::Index<ProgramState<Kind>> state;
    ygg::uint_t depth = 0;
    std::optional<ChoiceVariant> choice = std::nullopt;
    std::size_t next = no_entry;
};
}  // namespace detail

template<tyr::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program, const ProgramSearchOptions<Kind>& options) -> ProgramProofResults<Kind>
{
    using Step = detail::ProgramStep<Kind>;
    using Outcome = detail::ProgramOutcome;
    using Status = ProgramProofStatus;
    using SearchNode = detail::SearchNode<Kind>;
    using Predecessor = detail::Predecessor<Kind>;
    using ConceptChoice = detail::Choice<runir::kr::dl::ConceptTag>;
    using RoleChoice = detail::Choice<runir::kr::dl::RoleTag>;
    using Work = detail::Work<Kind>;
    using Choice = typename Work::ChoiceVariant;
    using Cursor = std::variant<typename ConceptChoice::Cursor, typename RoleChoice::Cursor>;
    struct ChoiceFrame
    {
        Work work;
        std::size_t shuffle_position;
        std::size_t num_bindings;
        std::size_t shuffle_begin;
        std::size_t open_head;
        std::size_t open_size;
        std::size_t visited_size;
        std::size_t edge_count;
        bool failed;
    };

    const auto& search_context = *task_context->search_context;
    const auto initial_node = search_context.successor_generator->get_packed_initial_node(*search_context.state_repository, *search_context.axiom_evaluator);
    auto result = ProgramProofResults<Kind> {};
    result.task_context_owner = task_context;
    auto predecessors = ygg::UnorderedSet<Predecessor> {};
    auto num_reached = std::size_t { 0 };
    auto initial_state = std::optional<ygg::Index<ProgramState<Kind>>> {};
    auto deadend_states = std::vector<ygg::Index<ProgramState<Kind>>> {};
    auto open_states = std::vector<ygg::Index<ProgramState<Kind>>> {};
    auto nodes = ygg::SegmentedVector<SearchNode> {};
    auto expander = SuccessorExpander<Kind>(task_context, program);
    auto classifier_caches = runir::kr::dl::semantics::DenotationCaches<runir::kr::UnsFamilyTag> {};
    auto expansions = std::vector<typename SuccessorExpander<Kind>::Expansion> {};
    auto shuffled_bindings = std::vector<Cursor> {};
    auto open = std::vector<Work> {};
    auto open_head = Work::no_entry;
    auto choices = std::vector<ChoiceFrame> {};
    auto visited = std::vector<ygg::Index<ProgramState<Kind>>> {};
    auto selected_edges = std::vector<std::tuple<ygg::Index<ProgramState<Kind>>, ygg::Index<ProgramState<Kind>>, ygg::uint_t>> {};
    auto failed = false;
    auto statistics = ProgramSearchStatistics {};
    const auto stopwatch = options.max_time ? std::optional(ygg::CountdownWatch(*options.max_time)) : std::nullopt;
    const auto out_of_time = [&] { return stopwatch && stopwatch->has_finished(); };

    auto attempt_analysis = detail::AttemptAnalysis<Kind> {};
    const auto finish = [&](Status status, ygg::uint_t choice_depth = 0)
    {
        if (status != Status::SUCCESS)
            attempt_analysis.assess(nodes.size(), selected_edges, statistics);
        result.status = status;
        detail::build_proof_graph(result,
                                  nodes,
                                  predecessors,
                                  initial_state,
                                  std::span<const ygg::Index<ProgramState<Kind>>>(deadend_states),
                                  std::span<const ygg::Index<ProgramState<Kind>>>(open_states));
        result.statistics = statistics;
        result.statistics.choice_depth = choice_depth;
        return std::move(result);
    };
    const auto state_view = [&](ygg::Index<ProgramState<Kind>> state) { return ProgramStateView<Kind>(state, *task_context->execution_repository); };
    const auto discover = [&](ProgramStateView<Kind> state) -> bool
    {
        auto& node = detail::get_or_create_search_node(state.get_index(), nodes);
        if (node.discovery_order != SearchNode::unreached)
            return true;
        if (num_reached >= options.max_num_states)
            return false;

        node.discovery_order = num_reached++;
        node.is_goal = expander.is_goal(state.get_state());
        if (!node.is_goal && options.classifier)
        {
            classifier_caches.clear(false);
            auto context = runir::kr::dl::semantics::StateEvaluationContext<runir::kr::UnsFamilyTag, Kind>(state.get_state(),
                                                                                                           task_context->dl_builder,
                                                                                                           *task_context->dl_denotation_repository,
                                                                                                           task_context->dl_builder.get_workspace(),
                                                                                                           classifier_caches);
            node.is_unsolvable = runir::kr::uns::classify(*options.classifier, context);
        }
        return true;
    };
    const auto push_work = [&](Work work)
    {
        work.next = open_head;
        open_head = open.size();
        open.push_back(work);
    };
    const auto enqueue = [&](ygg::Index<ProgramState<Kind>> state, ygg::uint_t depth = 0)
    {
        auto& node = detail::get_or_create_search_node(state, nodes);
        if (!node.visited)
        {
            node.visited = true;
            visited.push_back(state);
            push_work({ state, depth });
        }
    };
    const auto apply_step = [&](const Work& work, const Step& step, bool non_singleton = false) -> std::optional<Status>
    {
        const auto source = work.state;
        auto& node = detail::get_or_create_search_node(source, nodes);
        if (step.status == Outcome::OUT_OF_TIME)
            return Status::OUT_OF_TIME;
        if (step.status == Outcome::OUT_OF_STATES)
            return Status::OUT_OF_STATES;

        const auto choice = work.choice.has_value();
        const auto applied = step.status == Outcome::APPLIED || step.status == Outcome::RESTORED_CALLER;
        if (choice && applied)
            ++statistics.num_binding_attempts;
        const auto weight = ygg::uint_t(non_singleton);
        const auto depth = work.depth + weight;
        if (applied)
        {
            statistics.max_choice_depth = std::max(statistics.max_choice_depth, depth);
            const auto target = step.get_target().get_index();
            auto& target_node = detail::get_or_create_search_node(target, nodes);
            const auto known = target_node.discovery_order != SearchNode::unreached;
            if (!discover(step.get_target()))
                return Status::OUT_OF_STATES;
            target_node.boundary |= known;
            node.has_successor = true;
            const auto& transition = step.get_state_transition();
            predecessors.emplace(source,
                                 target,
                                 transition ? std::optional(transition->action) : std::nullopt,
                                 transition ? transition->cost : ygg::float_t(0),
                                 step.rule,
                                 predecessors.size());
            selected_edges.emplace_back(source, target, weight);
            if (!target_node.visited)
            {
                target_node.parent_state = source;
                target_node.action = step.planning_successor ? std::optional(step.planning_successor->label) : std::nullopt;
                target_node.metric = step.planning_successor ? step.planning_successor->node.get_metric() : 0;
            }
            enqueue(target, depth);
        }
        else if (choice)
        {
            // Failure belongs to this choose obligation, not to the shared source state.
            if (!node.reported_deadend)
                deadend_states.push_back(source);
            node.reported_deadend = true;
        }
        else
        {
            if (!node.reported_open)
                open_states.push_back(source);
            node.reported_open = true;
        }
        failed |= !applied;
        return std::nullopt;
    };

    const auto apply_choice = [&](ChoiceFrame& frame) -> std::optional<Status>
    {
        const auto step = std::visit(
            [&](auto& choice)
            {
                if (options.shuffle_choice_points && frame.num_bindings != 0)
                    choice.cursor = std::get<typename std::decay_t<decltype(choice)>::Cursor>(shuffled_bindings[frame.shuffle_begin + frame.shuffle_position]);
                return expander.apply_choice(state_view(frame.work.state), choice);
            },
            *frame.work.choice);
        statistics.num_generated += step.status == Outcome::APPLIED;
        return apply_step(frame.work, step, frame.num_bindings > 1);
    };

    const auto initial = expander.initial_state();
    if (!discover(initial))
        return finish(Status::OUT_OF_STATES);
    initial_state = initial.get_index();
    enqueue(*initial_state);

    while (true)
    {
        if (out_of_time())
            return finish(Status::OUT_OF_TIME);

        if (open_head == Work::no_entry)
        {
            const auto depth = attempt_analysis.assess(nodes.size(), selected_edges, statistics);
            if (!failed && depth)
                return finish(Status::SUCCESS, *depth);

            auto resumed = false;
            while (!choices.empty())
            {
                auto& frame = choices.back();
                if (frame.num_bindings != 0)
                    ++statistics.num_backtracks;
                open_head = frame.open_head;
                open.resize(frame.open_size);
                while (visited.size() > frame.visited_size)
                {
                    detail::get_or_create_search_node(visited.back(), nodes).visited = false;
                    visited.pop_back();
                }
                selected_edges.resize(frame.edge_count);
                failed = frame.failed;

                const auto has_next = frame.num_bindings != 0
                                      && (options.shuffle_choice_points ? ++frame.shuffle_position < frame.num_bindings :
                                                                          std::visit(
                                                                              [](auto& choice)
                                                                              {
                                                                                  choice.advance();
                                                                                  return !choice.exhausted();
                                                                              },
                                                                              *frame.work.choice));
                if (has_next)
                {
                    if (const auto limit = apply_choice(frame))
                        return finish(*limit);
                    resumed = true;
                    break;
                }
                shuffled_bindings.resize(frame.shuffle_begin);
                choices.pop_back();
            }
            if (!resumed)
                return finish(Status::FAILURE);
            continue;
        }

        const auto position = open_head;
        const auto work = open[position];
        open_head = work.next;
        // Entries protected by a live choice remain intact; later pushes are discarded on rollback.
        const auto protected_size = choices.empty() ? 0 : choices.back().open_size;
        if (position + 1 == open.size() && position >= protected_size)
            open.pop_back();
        if (work.choice)
        {
            const auto count = std::visit([](const auto& choice) { return choice.count(); }, *work.choice);
            const auto shuffle_begin = shuffled_bindings.size();
            if (options.shuffle_choice_points)
            {
                std::visit(
                    [&](const auto& choice)
                    {
                        for (auto cursor = choice.denotation.begin(); cursor != choice.denotation.end(); ++cursor)
                            shuffled_bindings.emplace_back(cursor);
                    },
                    *work.choice);
                auto random = std::mt19937_64(options.random_seed + detail::get_or_create_search_node(work.state, nodes).discovery_order);
                ygg::portable_shuffle(shuffled_bindings.begin() + shuffle_begin, shuffled_bindings.end(), random);
            }
            statistics.num_choice_points += count > 1;
            choices.push_back({ work, 0, count, shuffle_begin, open_head, open.size(), visited.size(), selected_edges.size(), failed });
            if (const auto limit = apply_choice(choices.back()))
                return finish(*limit);
            continue;
        }

        const auto state = state_view(work.state);
        auto& node = detail::get_or_create_search_node(work.state, nodes);
        if (node.is_goal)
        {
            if (!options.universal)
            {
                result.plan = detail::extract_total_ordered_plan(work.state, nodes, *task_context->execution_repository, initial_node);
                return finish(Status::SUCCESS, attempt_analysis.assess(nodes.size(), selected_edges, statistics).value());
            }
            continue;
        }
        if (node.is_unsolvable)
        {
            if (!node.reported_deadend)
                deadend_states.push_back(work.state);
            node.reported_deadend = true;
            failed = true;
            continue;
        }

        ++statistics.num_expanded;
        if (options.universal || options.shuffle_choice_points)
            expander.template expand_until<EagerExpansionPolicy>(state, out_of_time, expansions);
        else
            expander.template expand_until<LazyExpansionPolicy>(state, out_of_time, expansions);
        for (const auto& expansion : expansions)
            if (const auto* step = std::get_if<Step>(&expansion))
                statistics.num_generated += step->status == Outcome::APPLIED || step->status == Outcome::RESTORED_CALLER;
        if (out_of_time())
            return finish(Status::OUT_OF_TIME);
        if (options.shuffle_choice_points)
        {
            // Regenerating a state must preserve its ordering, independently of intervening retries.
            auto random = std::mt19937_64(options.random_seed + node.discovery_order);
            ygg::portable_shuffle(expansions.begin(), expansions.end(), random);
        }
        for (const auto& expansion : expansions)
        {
            if (out_of_time())
                return finish(Status::OUT_OF_TIME);
            const auto limit = std::visit(
                [&](const auto& value) -> std::optional<Status>
                {
                    if constexpr (std::same_as<std::decay_t<decltype(value)>, Step>)
                        return apply_step(work, value);
                    else
                    {
                        push_work({ work.state, work.depth, Choice(value) });
                        return std::nullopt;
                    }
                },
                expansion);
            if (limit)
                return finish(*limit);
            if (!options.universal)
                break;
        }
    }
}

}  // namespace runir::kr::ps::ext

#endif
