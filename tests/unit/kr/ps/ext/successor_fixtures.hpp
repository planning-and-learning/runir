#ifndef RUNIR_TESTS_EXT_SUCCESSOR_FIXTURES_HPP_
#define RUNIR_TESTS_EXT_SUCCESSOR_FIXTURES_HPP_

#include <concepts>
#include <functional>
#include <runir/kr/ps/ext/successor_expander.hpp>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace runir::tests
{

template<tyr::TaskKind Kind>
auto initial_planning_node(const kr::ps::ext::SuccessorExpander<Kind>& expander)
{
    auto& search = *expander.get_task_context()->search_context;
    return search.successor_generator->get_initial_node(*search.state_repository, *search.axiom_evaluator);
}

template<tyr::TaskKind Kind>
auto collect_steps(
    kr::ps::ext::SuccessorExpander<Kind>& expander,
    kr::ps::ext::ProgramStateView<Kind> state,
    bool first_only = false,
    const std::function<bool()>& stop = [] { return false; })
{
    using Expander = kr::ps::ext::SuccessorExpander<Kind>;
    auto statistics = kr::ps::ext::ProgramSearchStatistics {};
    auto expansions = std::vector<typename Expander::Expansion> {};
    expander.for_each_successor(
        state,
        statistics,
        [&](auto expansion)
        {
            expansions.push_back(std::move(expansion));
            return !first_only;
        },
        stop);
    auto result = std::vector<typename Expander::Step> {};
    for (auto& expansion : expansions)
        std::visit(
            [&](auto candidate)
            {
                if constexpr (std::same_as<decltype(candidate), typename Expander::Step>)
                    result.push_back(std::move(candidate));
                else if (candidate.exhausted())
                    result.push_back(expander.apply_choice(state, candidate, statistics));
                else
                    for (; !candidate.exhausted(); candidate.advance())
                        result.push_back(expander.apply_choice(state, candidate, statistics));
            },
            expansion);
    return result;
}

template<tyr::TaskKind Kind>
bool is_planning_goal(const kr::ps::ext::SuccessorExpander<Kind>& expander, const tyr::planning::StateView<Kind>& state)
{
    const auto& task = *expander.get_task_context()->search_context->task;
    auto goal = tyr::planning::ConjunctiveGoalStrategy<Kind>(task);
    return goal.is_static_goal_satisfied(task) && goal.is_dynamic_goal_satisfied(initial_planning_node(expander).get_state(), state);
}

}

#endif
