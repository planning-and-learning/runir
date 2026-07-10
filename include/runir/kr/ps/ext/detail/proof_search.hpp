#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/execution_state.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/proof_builder.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"

#include <utility>
#include <vector>

namespace runir::kr::ps::ext::detail
{

// Greedy execution: at each vertex drain the internal (Load) moves, then take the first control
// move the expander offers. Both step kinds come from `SuccessorExpander`, the single enumerator the
// prove search also uses, so greedy and prove can never disagree about what a rule does.
template<tyr::planning::TaskKind Kind>
auto execute_greedy_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                             runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                             ModuleProgramView program,
                             const ModuleExecutionOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    auto execution_state = ModuleProgramExecutionState<Kind>(search_context, program);
    auto& context = execution_state.get_context();
    const auto& initial_node = execution_state.get_initial_node();
    auto proof = ModuleProgramProofBuilder<Kind>(search_context, initial_node, program, std::move(context_owner));
    auto plan_steps = tyr::planning::LabeledNodeList<Kind> {};

    auto current_vertex = proof.get_or_create_vertex(context, ExternalMemoryState(context.get_call_stack().memory_state()), true, true, false).first;

    while (true)
    {
        if (proof.is_goal(context.get_state()))
        {
            proof.set_final_state(context.get_state());
            proof.set_plan(tyr::planning::Plan<Kind>(initial_node, std::move(plan_steps)));
            return proof.finish(ModuleProgramProofStatus::SUCCESS);
        }

        while (true)
        {
            const auto load_source = current_vertex;
            const auto load_steps = proof.load_steps(context);
            if (load_steps.empty())
                break;

            const auto& load_step = load_steps.front();
            const auto applied = load_step.status == ModuleProgramOutcome::APPLIED;
            context = load_step.context;

            const auto [target, created] =
                proof.get_or_create_vertex(context, InternalMemoryState(context.get_call_stack().memory_state()), false, applied, !applied);
            const auto edge = proof.add_edge(load_source, target, std::nullopt, load_step.rule);
            current_vertex = target;

            if (!applied)
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
            {
                proof.set_final_state(context.get_state());
                proof.set_plan(tyr::planning::Plan<Kind>(initial_node, std::move(plan_steps)));
                return proof.finish(ModuleProgramProofStatus::SUCCESS);
            }
        }

        const auto source_vertex = current_vertex;
        const auto control_steps = proof.control_steps(search_context, context, options);
        const auto& step = control_steps.front();

        if (step.status == ModuleProgramOutcome::APPLIED || step.status == ModuleProgramOutcome::RESTORED_CALLER)
        {
            context = step.context;
            plan_steps.insert(plan_steps.end(), step.plan_suffix.begin(), step.plan_suffix.end());
            const auto [target, created] =
                proof.get_or_create_vertex(context, ExternalMemoryState(context.get_call_stack().memory_state()), false, true, false);
            proof.add_edge(source_vertex, target, step.state_transition, step.rule);
            if (!created)
            {
                proof.set_two_vertex_cycle(target, source_vertex);
                return proof.finish(ModuleProgramProofStatus::FAILURE);
            }
            current_vertex = target;
            continue;
        }

        if (step.status == ModuleProgramOutcome::OUT_OF_TIME)
            return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
        if (step.status == ModuleProgramOutcome::OUT_OF_STATES)
            return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);

        proof.add_open_state(source_vertex);
        return proof.finish(translate_proof_status(step.status));
    }
}

template<tyr::planning::TaskKind Kind>
auto find_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                   runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                   ModuleProgramView program,
                   const ModuleExecutionOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    return execute_greedy_solution(search_context, std::move(context_owner), program, options);
}

// Proof search: from each open vertex, expand every internal (Load) move, or every control move
// when no Load applies, materializing each as a proof-graph edge + vertex. Same expander as greedy,
// so the proof graph contains exactly the moves greedy could have taken.
template<tyr::planning::TaskKind Kind>
auto prove_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                    runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                    ModuleProgramView program,
                    const ModuleExecutionOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    auto execution_state = ModuleProgramExecutionState<Kind>(search_context, program);
    auto proof = ModuleProgramProofBuilder<Kind>(search_context, execution_state.get_initial_node(), program, std::move(context_owner));
    auto open = std::vector<std::pair<EvaluationContext<Kind>, graphs::VertexIndex>> {};
    auto failed = false;

    auto& initial_context = execution_state.get_context();
    const auto initial_vertex =
        proof.get_or_create_vertex(initial_context, ExternalMemoryState(initial_context.get_call_stack().memory_state()), true, true, false).first;
    open.emplace_back(initial_context, initial_vertex);

    auto add_terminal = [&](graphs::VertexIndex source)
    {
        proof.add_open_state(source);
        failed = true;
    };

    auto add_successor = [&](graphs::VertexIndex source, const ModuleProgramStep<Kind>& step, MemoryStateVariant memory_state)
    {
        const auto [target, created] = proof.get_or_create_vertex(step.context, std::move(memory_state), false, true, false);
        proof.add_edge(source, target, step.state_transition, step.rule);
        if (created && !proof.is_goal(step.context.get_state()))
            open.emplace_back(step.context, target);
    };

    while (!open.empty())
    {
        auto [context, source_vertex] = std::move(open.back());
        open.pop_back();

        if (proof.is_goal(context.get_state()))
            continue;

        const auto load_steps = proof.load_steps(context);
        if (!load_steps.empty())
        {
            for (const auto& step : load_steps)
            {
                if (step.status == ModuleProgramOutcome::APPLIED)
                    add_successor(source_vertex, step, InternalMemoryState(step.context.get_call_stack().memory_state()));
                else
                    add_terminal(source_vertex);
            }
            continue;
        }

        const auto control_steps = proof.control_steps(search_context, context, options);
        for (const auto& step : control_steps)
        {
            if (step.status == ModuleProgramOutcome::APPLIED || step.status == ModuleProgramOutcome::RESTORED_CALLER)
                add_successor(source_vertex, step, ExternalMemoryState(step.context.get_call_stack().memory_state()));
            else if (step.status == ModuleProgramOutcome::OUT_OF_TIME)
                return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
            else if (step.status == ModuleProgramOutcome::OUT_OF_STATES)
                return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);
            else
                add_terminal(source_vertex);
        }
    }

    return proof.finish(failed ? ModuleProgramProofStatus::FAILURE : ModuleProgramProofStatus::SUCCESS);
}
}  // namespace runir::kr::ps::ext::detail

#endif
