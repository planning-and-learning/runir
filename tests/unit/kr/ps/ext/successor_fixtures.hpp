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
    using Step = kr::ps::ext::detail::ProgramStep<Kind>;
    using Expansion = std::variant<Step, kr::ps::ext::detail::Choice<kr::dl::ConceptTag>, kr::ps::ext::detail::Choice<kr::dl::RoleTag>>;
    auto statistics = kr::ps::ext::ProgramSearchStatistics {};
    auto expansions = std::vector<Expansion> {};
    expander.for_each_successor(
        state,
        statistics,
        [&](auto expansion)
        {
            expansions.push_back(std::move(expansion));
            return !first_only;
        },
        stop);
    auto result = std::vector<Step> {};
    for (auto& expansion : expansions)
        std::visit(
            [&](auto candidate)
            {
                if constexpr (std::same_as<decltype(candidate), Step>)
                    result.push_back(std::move(candidate));
                else if (candidate.exhausted())
                    result.push_back(expander.apply_choice(state, candidate, statistics));
                else
                    for (; !candidate.exhausted(); candidate.advance())
                        result.push_back(expander.apply_choice(state, candidate, statistics));
            },
            std::move(expansion));
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
