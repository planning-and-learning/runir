#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/binding_order.hpp"
#include "runir/kr/ps/ext/detail/attempt_analysis.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/proof_graph.hpp"
#include "runir/kr/ps/ext/detail/search_space.hpp"
#include "runir/kr/ps/ext/program_executor.hpp"
#include "runir/kr/ps/ext/successor_expander.hpp"
#include "runir/kr/ps/unsolvability.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <limits>
#include <optional>
#include <random>
#include <tuple>
#include <type_traits>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/containers/segmented_vector.hpp>
#include <yggdrasil/containers/unordered_set.hpp>
#include <yggdrasil/core/chrono.hpp>
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
    tyr::planning::PackedNode<Kind> node;
    ygg::uint_t depth = 0;
    std::optional<ChoiceVariant> choice = std::nullopt;
    std::size_t next = no_entry;
};

template<tyr::TaskKind Kind, typename BindingOrder, typename Unsolvability>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context,
                   ProgramView program,
                   const ProgramSearchOptions<Kind>& options,
                   BindingOrder order,
                   Unsolvability& classifier) -> ProgramProofResults<Kind>
{
    using Step = detail::ProgramStep<Kind>;
    using Outcome = detail::ProgramOutcome;
    using Status = ProgramProofStatus;
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
    const auto initial_planning_state = initial_node.get_state().unpack();
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*search_context.task);
    const auto static_goal_satisfied = goal_strategy.is_static_goal_satisfied(*search_context.task);
    auto result = ProgramProofResults<Kind> {};
    result.task_context_owner = task_context;
    auto predecessors = ygg::UnorderedSet<detail::Predecessor<Kind>> {};
    auto num_reached = std::size_t { 0 };
    auto initial_state = std::optional<ygg::Index<ProgramState<Kind>>> {};
    auto nodes = ygg::SegmentedVector<detail::SearchNode<Kind>> {};
    auto expander = SuccessorExpander<Kind>(task_context, program);
    auto shuffled_bindings = std::conditional_t<BindingOrder::shuffled, std::vector<Cursor>, std::monostate> {};
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
        detail::build_proof_graph(result, nodes, predecessors, initial_state);
        result.statistics = statistics;
        result.statistics.choice_depth = choice_depth;
        return std::move(result);
    };
    const auto state_view = [&](ygg::Index<ProgramState<Kind>> state) { return ProgramStateView<Kind>(state, *task_context->execution_repository); };
    const auto discover = [&](ProgramStateView<Kind> state) -> bool
    {
        auto& node = detail::get_or_create_search_node(state.get_index(), nodes);
        if (node.step != detail::SearchNode<Kind>::unreached)
            return true;
        if (num_reached >= options.max_num_states)
            return false;

        node.step = num_reached++;
        const auto planning_state = state.get_state();
        node.is_goal = static_goal_satisfied && goal_strategy.is_dynamic_goal_satisfied(initial_planning_state, planning_state);
        node.is_unsolvable = !node.is_goal && classifier.is_unsolvable(planning_state);
        return true;
    };
    const auto push_work = [&](Work work)
    {
        work.next = open_head;
        open_head = open.size();
        open.push_back(work);
    };
    const auto enqueue = [&](ygg::Index<ProgramState<Kind>> state, const tyr::planning::PackedNode<Kind>& planning_node, ygg::uint_t depth = 0)
    {
        auto& node = detail::get_or_create_search_node(state, nodes);
        if (!node.visited)
        {
            node.visited = true;
            visited.push_back(state);
            push_work({ state, planning_node, depth });
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
            if (!discover(step.get_target()))
                return Status::OUT_OF_STATES;
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
                target_node.planning_successor = step.planning_successor;
            }
            enqueue(target, step.planning_successor ? step.planning_successor->node : work.node, depth);
        }
        else if (choice)
        {
            // Failure belongs to this choose obligation, not to the shared source state.
            node.is_deadend = true;
        }
        else
        {
            node.is_open = true;
        }
        failed |= !applied;
        return std::nullopt;
    };

    const auto apply_choice = [&](ChoiceFrame& frame) -> std::optional<Status>
    {
        const auto step = std::visit(
            [&](auto& choice)
            {
                if constexpr (BindingOrder::shuffled)
                    if (frame.num_bindings != 0)
                        choice.cursor =
                            std::get<typename std::decay_t<decltype(choice)>::Cursor>(shuffled_bindings[frame.shuffle_begin + frame.shuffle_position]);
                return expander.apply_choice(state_view(frame.work.state), frame.work.node.unpack(), choice, statistics);
            },
            *frame.work.choice);
        return apply_step(frame.work, step, frame.num_bindings > 1);
    };

    const auto initial = expander.initial_state(initial_node.unpack());
    if (!discover(initial))
        return finish(Status::OUT_OF_STATES);
    initial_state = initial.get_index();
    enqueue(*initial_state, initial_node);

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
                open.erase(open.begin() + frame.open_size, open.end());
                while (visited.size() > frame.visited_size)
                {
                    detail::get_or_create_search_node(visited.back(), nodes).visited = false;
                    visited.pop_back();
                }
                selected_edges.resize(frame.edge_count);
                failed = frame.failed;

                auto has_next = false;
                if (frame.num_bindings != 0)
                {
                    if constexpr (BindingOrder::shuffled)
                        has_next = ++frame.shuffle_position < frame.num_bindings;
                    else
                        has_next = std::visit(
                            [](auto& choice)
                            {
                                choice.advance();
                                return !choice.exhausted();
                            },
                            *frame.work.choice);
                }
                if (has_next)
                {
                    if (const auto limit = apply_choice(frame))
                        return finish(*limit);
                    resumed = true;
                    break;
                }
                if constexpr (BindingOrder::shuffled)
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
            auto shuffle_begin = std::size_t { 0 };
            if constexpr (BindingOrder::shuffled)
            {
                shuffle_begin = shuffled_bindings.size();
                std::visit(
                    [&](const auto& choice)
                    {
                        for (auto cursor = choice.denotation.begin(); cursor != choice.denotation.end(); ++cursor)
                        {
                            if (out_of_time())
                                return;
                            shuffled_bindings.emplace_back(cursor);
                        }
                    },
                    *work.choice);
                if (out_of_time())
                    return finish(Status::OUT_OF_TIME);
                order.reset(options.random_seed + detail::get_or_create_search_node(work.state, nodes).step);
                order.shuffle(shuffled_bindings.begin() + shuffle_begin, shuffled_bindings.end());
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
                result.plan = detail::extract_total_ordered_plan(work.state, nodes, initial_node);
                return finish(Status::SUCCESS, attempt_analysis.assess(nodes.size(), selected_edges, statistics).value());
            }
            continue;
        }
        if (node.is_unsolvable)
        {
            node.is_deadend = true;
            failed = true;
            continue;
        }

        ++statistics.num_expanded;
        // Regenerating a state must preserve its ordering, independently of intervening retries.
        order.reset(options.random_seed + node.step);
        auto limit = std::optional<Status> {};
        expander.for_each_successor(
            state,
            work.node.unpack(),
            statistics,
            order,
            [&](const typename SuccessorExpander<Kind>::Expansion& expansion)
            {
                if (out_of_time())
                {
                    limit = Status::OUT_OF_TIME;
                    return false;
                }
                limit = std::visit(
                    [&](const auto& value) -> std::optional<Status>
                    {
                        if constexpr (std::same_as<std::decay_t<decltype(value)>, Step>)
                            return apply_step(work, value);
                        else
                        {
                            push_work({ work.state, work.node, work.depth, Choice(value) });
                            return std::nullopt;
                        }
                    },
                    expansion);
                return !limit && options.universal;
            },
            out_of_time);
        if (limit)
            return finish(*limit);
        if (out_of_time())
            return finish(Status::OUT_OF_TIME);
    }
}

}  // namespace detail

template<tyr::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context_owner,
                   ProgramView program,
                   const ProgramSearchOptions<Kind>& options) -> ProgramProofResults<Kind>
{
    const auto search = [&](auto& classifier)
    {
        // Retain the task until the classifier's caches are destroyed, including on exceptions.
        if (options.shuffle_choice_points)
        {
            auto random = std::mt19937_64(options.random_seed);
            return detail::find_solution(task_context_owner, program, options, Shuffled(random), classifier);
        }
        return detail::find_solution(task_context_owner, program, options, InOrder {}, classifier);
    };
    if (options.classifier)
    {
        auto classifier = ClassifierUnsolvability<Kind>(*task_context_owner, *options.classifier);
        return search(classifier);
    }
    auto classifier = NoUnsolvability {};
    return search(classifier);
}

}  // namespace runir::kr::ps::ext

#endif
