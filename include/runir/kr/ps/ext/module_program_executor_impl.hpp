#ifndef RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_IMPL_HPP_
#define RUNIR_KR_PS_EXT_MODULE_PROGRAM_EXECUTOR_IMPL_HPP_

#include "runir/kr/ps/ext/detail/configuration.hpp"
#include "runir/kr/ps/ext/detail/execution_state.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/plan_trace.hpp"
#include "runir/kr/ps/ext/detail/proof_builder.hpp"
#include "runir/kr/ps/ext/detail/transition_search.hpp"
#include "runir/kr/ps/ext/execution.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"

#include <optional>
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
auto find_solution(const runir::datasets::TaskSearchContext<Kind>& context,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options) -> tyr::planning::SearchResult<Kind>
{
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
auto prove_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                    ModuleProgramView program,
                    const ModuleProgramSearchOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    auto execution_state = detail::ModuleProgramExecutionState<Kind>(search_context, program);
    auto& context = execution_state.get_context();
    auto proof = detail::ModuleProgramProofBuilder<Kind>(search_context, execution_state.get_initial_node(), context.get_repository_owner());
    const auto internal_options = detail::execution_options(options);

    auto current_vertex = proof.get_or_create_vertex(context, true, true, false).first;

    while (true)
    {
        if (proof.is_goal(context.get_state()))
            return proof.finish(ModuleProgramProofStatus::SUCCESS);

        while (true)
        {
            const auto load_source = current_vertex;
            const auto load_status = execute_next_load(context);
            if (load_status == LoadExecutionStatus::STABLE)
                break;

            const auto terminal = load_status != LoadExecutionStatus::APPLIED;
            const auto [target, created] = proof.get_or_create_vertex(context, false, !terminal, terminal);
            const auto edge = proof.add_edge(load_source, target, std::nullopt, 0);
            current_vertex = target;

            if (load_status == LoadExecutionStatus::EMPTY_DENOTATION)
            {
                proof.add_deadend_transition(edge);
                proof.add_open_state(target);
                return proof.finish(ModuleProgramProofStatus::FAILURE);
            }

            if (!created)
            {
                proof.set_two_vertex_cycle(target, load_source);
                return proof.finish(ModuleProgramProofStatus::FAILURE);
            }

            if (proof.is_goal(context.get_state()))
                return proof.finish(ModuleProgramProofStatus::SUCCESS);
        }

        const auto source_vertex = current_vertex;
        const auto step_result = detail::execute_next_control_step(search_context, context, internal_options);

        if (step_result.status == detail::ModuleProgramOutcome::APPLIED || step_result.status == detail::ModuleProgramOutcome::RESTORED_CALLER)
        {
            const auto [target, created] = proof.get_or_create_vertex(context, false, true, false);
            proof.add_edge(source_vertex, target, step_result.action, step_result.cost, step_result.rule);
            if (!created)
            {
                proof.set_two_vertex_cycle(target, source_vertex);
                return proof.finish(ModuleProgramProofStatus::FAILURE);
            }
            current_vertex = target;
            continue;
        }

        if (step_result.status == detail::ModuleProgramOutcome::OUT_OF_TIME)
            return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
        if (step_result.status == detail::ModuleProgramOutcome::OUT_OF_STATES)
            return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);

        const auto [target, created] = proof.get_or_create_vertex(context, false, false, true);
        static_cast<void>(created);
        const auto edge = proof.add_edge(source_vertex, target, std::nullopt, 0);
        proof.add_deadend_transition(edge);
        proof.add_open_state(target);
        return proof.finish(detail::translate_proof_status(step_result.status));
    }
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
    auto num_steps = std::size_t(0);

    auto finish = [&](ModuleProgramOutcome status)
    {
        return ModuleExecutionResults<Kind> { status,       context.get_state(), context.get_module(), context.get_memory_state(), context.get_repository_owner(),
                                              std::nullopt, num_steps,           context.get_call_stack().size() };
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

            ++num_steps;
            if (!history.remember(context))
                return finish(ModuleProgramOutcome::CYCLE);
        }

        const auto step_result = execute_next_control_step(search_context, context, options);
        if (step_result.status == ModuleProgramOutcome::APPLIED || step_result.status == ModuleProgramOutcome::RESTORED_CALLER)
        {
            plan_steps.insert(plan_steps.end(), step_result.plan_suffix.begin(), step_result.plan_suffix.end());
            ++num_steps;
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
