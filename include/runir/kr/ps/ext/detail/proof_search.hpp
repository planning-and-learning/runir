#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/proof_builder.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"
#include "runir/kr/task_context.hpp"

#include <chrono>
#include <random>
#include <tyr/planning/algorithms/portable_shuffle.hpp>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
auto find_solution(runir::kr::TaskContextPtr<Kind> task_context,
                   ModuleProgramView program,
                   const ModuleProgramSearchOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    const auto initial_node = task_context->search_context->successor_generator->get_initial_node();
    auto proof = ModuleProgramProofBuilder<Kind>(std::move(task_context), program, options.classifier);
    auto open = std::vector<std::pair<ExecutionStateView<Kind>, graphs::VertexIndex>> {};
    auto plan_steps = tyr::planning::LabeledNodeList<Kind> {};
    auto failed = false;

    const auto initial_state = proof.initial_state();
    const auto initial_result = proof.get_or_create_vertex(initial_state, true, true, false, options.max_num_states);
    if (!initial_result)
        return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);
    open.emplace_back(initial_state, initial_result->first);

    const auto started_at = std::chrono::steady_clock::now();
    auto random = std::mt19937_64(options.random_seed);
    const auto out_of_time = [&]() { return options.max_time && std::chrono::steady_clock::now() - started_at >= *options.max_time; };

    while (!open.empty())
    {
        if (out_of_time())
            return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);

        const auto [state, source_vertex] = open.back();
        open.pop_back();

        if (proof.is_goal(state.get_state()))
        {
            if (!options.universal)
            {
                proof.set_plan(tyr::planning::Plan<Kind>(initial_node, std::move(plan_steps)));
                return proof.finish(ModuleProgramProofStatus::SUCCESS);
            }
            continue;
        }
        if (proof.is_unsolvable(source_vertex))
        {
            proof.add_deadend_state(source_vertex);
            failed = true;
            continue;
        }

        auto load_steps = proof.load_steps(state, out_of_time);
        if (out_of_time())
            return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
        if (!load_steps.empty())
        {
            if (options.shuffle_choice_points)
                tyr::planning::portable_shuffle(load_steps.begin(), load_steps.end(), random);
            if (!options.universal && load_steps.size() > 1)
                load_steps.erase(load_steps.begin() + 1, load_steps.end());

            for (const auto& step : load_steps)
            {
                if (out_of_time())
                    return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);

                const auto applied = step.status == ModuleProgramOutcome::APPLIED;
                const auto target_state = step.get_target();
                const auto target_result = proof.get_or_create_vertex(target_state, false, applied, !applied, options.max_num_states);
                if (!target_result)
                    return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);

                const auto [target, created] = *target_result;
                proof.add_edge(source_vertex, target, step.get_state_transition(), step.rule);
                if (!applied)
                {
                    proof.add_deadend_state(target);
                    failed = true;
                }
                else if (created)
                {
                    open.emplace_back(target_state, target);
                }
            }
            continue;
        }

        auto successors = proof.labeled_successors(state);
        if (out_of_time())
            return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
        if (options.shuffle_labeled_succ_nodes)
            tyr::planning::portable_shuffle(successors.begin(), successors.end(), random);

        auto control_steps = proof.control_steps(state, successors, out_of_time);
        if (out_of_time())
            return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
        if (options.shuffle_choice_points)
            tyr::planning::portable_shuffle(control_steps.begin(), control_steps.end(), random);
        if (!options.universal && control_steps.size() > 1)
            control_steps.erase(control_steps.begin() + 1, control_steps.end());

        for (const auto& step : control_steps)
        {
            if (out_of_time())
                return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);

            if (step.status == ModuleProgramOutcome::APPLIED || step.status == ModuleProgramOutcome::RESTORED_CALLER)
            {
                const auto target_state = step.get_target();
                const auto target_result = proof.get_or_create_vertex(target_state, false, true, false, options.max_num_states);
                if (!target_result)
                    return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);

                const auto [target, created] = *target_result;
                proof.add_edge(source_vertex, target, step.get_state_transition(), step.rule);
                if (!options.universal)
                    plan_steps.insert(plan_steps.end(), step.plan_suffix.begin(), step.plan_suffix.end());
                if (created)
                    open.emplace_back(target_state, target);
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
