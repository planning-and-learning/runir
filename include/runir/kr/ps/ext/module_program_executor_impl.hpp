#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_IMPL_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_IMPL_HPP_

#include "runir/kr/ps/ext/detail/configuration.hpp"
#include "runir/kr/ps/ext/detail/execution_state.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/proof_search.hpp"
#include "runir/kr/ps/ext/detail/transition_search.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"

#include <optional>
#include <utility>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/successor_generator.hpp>

namespace runir::kr::ps::ext
{
namespace detail
{

template<tyr::planning::TaskKind Kind>
auto execute_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      ModuleProgramView program,
                      const ModuleExecutionOptions<Kind>& options) -> ModuleExecutionResults<Kind>;

}  // namespace detail

template<tyr::planning::TaskKind Kind>
auto find_solution(runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options) -> tyr::planning::SearchResult<Kind>
{
    const auto& context = *context_owner;
    const auto execution = detail::execute_solution(context, program, detail::execution_options(options));

    auto result = tyr::planning::SearchResult<Kind> {};
    result.status = detail::translate_execution_status(execution.status);
    if (execution.status == detail::ModuleProgramOutcome::SUCCESS)
    {
        result.goal_node = context.successor_generator->get_node(execution.state.get_index());
        result.plan = execution.plan;
    }
    return result;
}

template<tyr::planning::TaskKind Kind>
auto prove_solution(runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                    ModuleProgramView program,
                    const ModuleProgramSearchOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    const auto& search_context = *context_owner;
    return detail::prove_solution(search_context, std::move(context_owner), program, detail::execution_options(options));
}

namespace detail
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
    history.remember(context);
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
            const auto load_status = detail::execute_next_load(context);
            if (load_status == detail::LoadExecutionStatus::STABLE)
                break;
            if (load_status == detail::LoadExecutionStatus::EMPTY_DENOTATION)
                return finish(ModuleProgramOutcome::FAILURE);

            if (!history.remember(context))
                return finish(ModuleProgramOutcome::CYCLE);
        }

        const auto step_result = execute_next_control_step(search_context, context, options);
        if (step_result.status == ModuleProgramOutcome::APPLIED || step_result.status == ModuleProgramOutcome::RESTORED_CALLER)
        {
            plan_steps.insert(plan_steps.end(), step_result.plan_suffix.begin(), step_result.plan_suffix.end());
            if (!history.remember(context))
                return finish(ModuleProgramOutcome::CYCLE);
            continue;
        }

        return finish(step_result.status);
    }
}

}  // namespace detail

}  // namespace runir::kr::ps::ext

#endif
