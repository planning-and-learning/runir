#ifndef RUNIR_KR_PS_EXT_DETAIL_SOLUTION_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_SOLUTION_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/configuration.hpp"
#include "runir/kr/ps/ext/detail/execution_state.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/transition_search.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"

#include <optional>
#include <utility>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/node.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
auto execute_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      ModuleProgramView program,
                      const ModuleExecutionOptions<Kind>& options) -> ModuleExecutionResults<Kind>
{
    auto execution_state = ModuleProgramExecutionState<Kind>(search_context, program);
    auto& context = execution_state.get_context();
    const auto& initial_node = execution_state.get_initial_node();
    auto plan_steps = tyr::planning::LabeledNodeList<Kind> {};
    auto history = ModuleProgramConfigurationHistory<Kind> {};
    history.remember(context, ExternalMemoryState(context.get_memory_state()));
    auto goal_strategy = tyr::planning::ConjunctiveGoalStrategy<Kind>(*search_context.task);
    const auto initial_state = context.get_state();
    const auto static_goal_satisfied = goal_strategy.is_static_goal_satisfied(*search_context.task);
    auto finish = [&](ModuleProgramOutcome status)
    {
        return ModuleExecutionResults<Kind> { status, context.get_state(), std::nullopt };
    };

    while (true)
    {
        if (static_goal_satisfied && goal_strategy.is_dynamic_goal_satisfied(initial_state, context.get_state()))
        {
            auto result = finish(ModuleProgramOutcome::SUCCESS);
            result.plan = tyr::planning::Plan<Kind>(initial_node, std::move(plan_steps));
            return result;
        }

        while (true)
        {
            const auto load_status = execute_next_load(context);
            if (load_status == LoadExecutionStatus::STABLE)
                break;
            if (load_status == LoadExecutionStatus::EMPTY_DENOTATION)
                return finish(ModuleProgramOutcome::FAILURE);

            if (!history.remember(context, InternalMemoryState(context.get_memory_state())))
                return finish(ModuleProgramOutcome::CYCLE);
        }

        const auto step_result = execute_next_control_step(search_context, context, options);
        if (step_result.status == ModuleProgramOutcome::APPLIED || step_result.status == ModuleProgramOutcome::RESTORED_CALLER)
        {
            plan_steps.insert(plan_steps.end(), step_result.plan_suffix.begin(), step_result.plan_suffix.end());
            if (!history.remember(context, ExternalMemoryState(context.get_memory_state())))
                return finish(ModuleProgramOutcome::CYCLE);
            continue;
        }

        return finish(step_result.status);
    }
}

}  // namespace runir::kr::ps::ext::detail

#endif
