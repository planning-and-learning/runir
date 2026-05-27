#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/execution_state.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/proof_builder.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"

#include <optional>
#include <utility>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::TaskKind Kind>
auto prove_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                    runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                    ModuleProgramView program,
                    const ModuleExecutionOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    auto execution_state = ModuleProgramExecutionState<Kind>(search_context, program);
    auto& context = execution_state.get_context();
    auto proof = ModuleProgramProofBuilder<Kind>(search_context, execution_state.get_initial_node(), std::move(context_owner));

    auto current_vertex = proof.get_or_create_vertex(context, ExternalMemoryState(context.get_memory_state()), true, true, false).first;

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
            const auto [target, created] = proof.get_or_create_vertex(context, InternalMemoryState(context.get_memory_state()), false, !terminal, terminal);
            const auto edge = proof.add_edge(load_source, target, std::nullopt);
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
        const auto step_result = execute_next_control_step(search_context, context, options);

        if (step_result.status == ModuleProgramOutcome::APPLIED || step_result.status == ModuleProgramOutcome::RESTORED_CALLER)
        {
            const auto [target, created] = proof.get_or_create_vertex(context, ExternalMemoryState(context.get_memory_state()), false, true, false);
            proof.add_edge(source_vertex, target, step_result.state_transition, step_result.rule);
            if (!created)
            {
                proof.set_two_vertex_cycle(target, source_vertex);
                return proof.finish(ModuleProgramProofStatus::FAILURE);
            }
            current_vertex = target;
            continue;
        }

        if (step_result.status == ModuleProgramOutcome::OUT_OF_TIME)
            return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
        if (step_result.status == ModuleProgramOutcome::OUT_OF_STATES)
            return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);

        const auto [target, created] = proof.get_or_create_vertex(context, ExternalMemoryState(context.get_memory_state()), false, false, true);
        static_cast<void>(created);
        const auto edge = proof.add_edge(source_vertex, target, std::nullopt);
        proof.add_deadend_transition(edge);
        proof.add_open_state(target);
        return proof.finish(translate_proof_status(step_result.status));
    }
}

}  // namespace runir::kr::ps::ext::detail

#endif
