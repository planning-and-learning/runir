#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/execution_state.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/proof_builder.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"

#include <algorithm>
#include <chrono>
#include <random>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
auto find_solution(runir::kr::TaskContext<Kind>& task_context,
                   runir::kr::TaskContextPtr<Kind> task_context_owner,
                   ModuleProgramView program,
                   bool universal,
                   const ModuleProgramSearchOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    auto& search_context = *task_context.search_context;
    auto execution_state = ModuleProgramExecutionState<Kind>(search_context, program);
    const auto& initial_node = execution_state.get_initial_node();
    auto proof = ModuleProgramProofBuilder<Kind>(task_context, initial_node, program, std::move(task_context_owner));
    auto open = std::vector<std::pair<EvaluationContext<Kind>, graphs::VertexIndex>> {};
    auto plan_steps = tyr::planning::LabeledNodeList<Kind> {};
    auto failed = false;

    auto& initial_context = execution_state.get_context();
    const auto initial_result = proof.get_or_create_vertex(initial_context,
                                                           ExternalMemoryState(initial_context.get_call_stack().memory_state()),
                                                           true,
                                                           true,
                                                           false,
                                                           options.max_num_states);
    if (!initial_result)
        return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);
    open.emplace_back(initial_context, initial_result->first);

    const auto started_at = std::chrono::steady_clock::now();
    auto random = std::mt19937_64(options.random_seed);
    const auto out_of_time = [&]() { return options.max_time && std::chrono::steady_clock::now() - started_at >= *options.max_time; };

    while (!open.empty())
    {
        if (out_of_time())
            return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);

        auto [context, source_vertex] = std::move(open.back());
        open.pop_back();

        if (proof.is_goal(context.get_state()))
        {
            if (!universal)
            {
                proof.set_final_state(context.get_state());
                proof.set_plan(tyr::planning::Plan<Kind>(initial_node, std::move(plan_steps)));
                return proof.finish(ModuleProgramProofStatus::SUCCESS);
            }
            continue;
        }

        auto load_steps = proof.load_steps(context, out_of_time);
        if (out_of_time())
            return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
        if (!load_steps.empty())
        {
            if (!universal)
                load_steps.erase(load_steps.begin() + 1, load_steps.end());

            for (const auto& step : load_steps)
            {
                if (out_of_time())
                    return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);

                const auto applied = step.status == ModuleProgramOutcome::APPLIED;
                const auto target_result = proof.get_or_create_vertex(step.context,
                                                                      InternalMemoryState(step.context.get_call_stack().memory_state()),
                                                                      false,
                                                                      applied,
                                                                      !applied,
                                                                      options.max_num_states);
                if (!target_result)
                    return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);

                const auto [target, created] = *target_result;
                const auto edge = proof.add_edge(source_vertex, target, step.state_transition, step.rule);
                if (!applied)
                {
                    proof.add_deadend_transition(edge);
                    proof.add_open_state(target);
                    failed = true;
                }
                else if (created)
                {
                    open.emplace_back(step.context, target);
                }
            }
            continue;
        }

        auto successors = proof.labeled_successors(context);
        if (out_of_time())
            return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
        if (options.shuffle_labeled_succ_nodes)
            std::shuffle(successors.begin(), successors.end(), random);

        auto control_steps = proof.control_steps(context, successors, out_of_time);
        if (out_of_time())
            return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
        if (!universal)
            control_steps.erase(control_steps.begin() + 1, control_steps.end());

        for (const auto& step : control_steps)
        {
            if (out_of_time())
                return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);

            if (step.status == ModuleProgramOutcome::APPLIED || step.status == ModuleProgramOutcome::RESTORED_CALLER)
            {
                const auto target_result = proof.get_or_create_vertex(step.context,
                                                                      ExternalMemoryState(step.context.get_call_stack().memory_state()),
                                                                      false,
                                                                      true,
                                                                      false,
                                                                      options.max_num_states);
                if (!target_result)
                    return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);

                const auto [target, created] = *target_result;
                proof.add_edge(source_vertex, target, step.state_transition, step.rule);
                if (!universal)
                    plan_steps.insert(plan_steps.end(), step.plan_suffix.begin(), step.plan_suffix.end());
                if (created)
                    open.emplace_back(step.context, target);
            }
            else if (step.status == ModuleProgramOutcome::OUT_OF_TIME)
            {
                return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
            }
            else if (step.status == ModuleProgramOutcome::OUT_OF_STATES)
            {
                return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);
            }
            else
            {
                proof.add_open_state(source_vertex);
                failed = true;
            }
        }
    }

    return proof.finish(failed ? ModuleProgramProofStatus::FAILURE : ModuleProgramProofStatus::SUCCESS);
}

}  // namespace runir::kr::ps::ext::detail

#endif
