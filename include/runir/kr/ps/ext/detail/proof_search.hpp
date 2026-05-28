#ifndef RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_PROOF_SEARCH_HPP_

#include "runir/kr/ps/ext/detail/execution_state.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/proof_builder.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"

#include <optional>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext::detail
{


template<tyr::planning::TaskKind Kind>
struct ModuleProofStep
{
    ModuleProgramOutcome status;
    EvaluationContext<Kind> context;
    std::optional<datasets::StateGraphEdgeLabel> state_transition = std::nullopt;
    std::optional<RuleVariantView> rule = std::nullopt;
    tyr::planning::LabeledNodeList<Kind> plan_suffix;

    ModuleProofStep(ModuleProgramOutcome status_, EvaluationContext<Kind> context_) : status(status_), context(std::move(context_)) {}
};

template<tyr::planning::TaskKind Kind>
auto enumerate_load_steps(const EvaluationContext<Kind>& context) -> std::vector<ModuleProofStep<Kind>>
{
    auto result = std::vector<ModuleProofStep<Kind>> {};
    const auto module = context.get_module();
    for (const auto& transition : module.get_memory_transitions())
    {
        if (transition.source != context.get_memory_state().get_index())
            continue;

        for (auto rule_variant : tyr::make_view(transition.rules, context.get_repository()))
        {
            tyr::visit(
                [&](auto rule)
                {
                    using RuleView = std::decay_t<decltype(rule)>;
                    if constexpr (std::same_as<RuleView, tyr::View<tyr::Index<Rule<LoadTag>>, Repository>>)
                    {
                        if (!has_current_source(rule, context))
                            return;

                        auto eval_context = context;
                        if (!conditions_are_compatible(rule, eval_context))
                            return;

                        const auto denotation = evaluate(rule.get_concept(), eval_context);
                        if (denotation.begin() == denotation.end())
                        {
                            result.emplace_back(ModuleProgramOutcome::FAILURE, context);
                            return;
                        }

                        auto target = context;
                        target.set(rule.get_register().get_identifier(), (*denotation.begin()).get_index());
                        target.set_memory_state(rule.get_target());
                        result.emplace_back(ModuleProgramOutcome::APPLIED, std::move(target));
                    }
                },
                rule_variant.get_variant());
        }
    }
    return result;
}

template<tyr::planning::TaskKind Kind>
auto make_do_step(const EvaluationContext<Kind>& context,
                  RuleVariantView rule_variant,
                  RuleView<DoTag> rule,
                  const tyr::planning::StateView<Kind>& source_state,
                  const tyr::planning::LabeledNode<Kind>& successor) -> ModuleProofStep<Kind>
{
    auto target = context;
    target.set_state(successor.node.get_state());
    target.set_memory_state(rule.get_target());
    auto step = ModuleProofStep<Kind>(ModuleProgramOutcome::APPLIED, std::move(target));
    step.rule = rule_variant;
    step.plan_suffix.push_back(successor);
    const auto cost = source_state.get_index() == successor.node.get_state().get_index() ? tyr::float_t(0) : tyr::float_t(1);
    step.state_transition = datasets::StateGraphEdgeLabel { successor.label, cost };
    return step;
}

template<tyr::planning::TaskKind Kind>
auto enumerate_immediate_control_steps(const runir::datasets::TaskSearchContext<Kind>& search_context, const EvaluationContext<Kind>& context)
    -> std::vector<ModuleProofStep<Kind>>
{
    auto result = std::vector<ModuleProofStep<Kind>> {};
    const auto source_state = context.get_state();
    const auto node = search_context.successor_generator->get_node(source_state.get_index());
    const auto successors = search_context.successor_generator->get_labeled_successor_nodes(node);
    const auto module = context.get_module();

    for (const auto& transition : module.get_memory_transitions())
    {
        if (transition.source != context.get_memory_state().get_index())
            continue;

        for (auto rule_variant : tyr::make_view(transition.rules, context.get_repository()))
        {
            tyr::visit(
                [&](auto rule)
                {
                    using RuleViewT = std::decay_t<decltype(rule)>;
                    if constexpr (std::same_as<RuleViewT, RuleView<DoTag>>)
                    {
                        if (!has_current_source(rule, context))
                            return;

                        auto eval_context = context;
                        if (!conditions_are_compatible(rule, eval_context))
                            return;

                        if (const auto successor = select_do_successor(rule, eval_context, successors))
                        {
                            result.push_back(make_do_step(context, rule_variant, rule, source_state, *successor));
                        }
                        else
                        {
                            auto target = context;
                            if (target.restore_caller())
                                result.emplace_back(ModuleProgramOutcome::RESTORED_CALLER, std::move(target));
                            else
                                result.emplace_back(ModuleProgramOutcome::NO_APPLICABLE_ACTION, context);
                        }
                    }
                    else if constexpr (std::same_as<RuleViewT, RuleView<CallTag>>)
                    {
                        if (!has_current_source(rule, context))
                            return;

                        auto eval_context = context;
                        if (!conditions_are_compatible(rule, eval_context))
                            return;

                        auto arguments = evaluate_call_arguments(rule, eval_context);
                        const auto callee = resolve_callee(rule, context);
                        if (!callee || !call_arguments_match_signature(*callee, arguments))
                        {
                            result.emplace_back(ModuleProgramOutcome::MALFORMED_CALL, context);
                            return;
                        }

                        auto target = context;
                        target.enter_module(*callee,
                                            rule.get_target(),
                                            std::move(arguments.concept_arguments),
                                            std::move(arguments.role_arguments),
                                            std::move(arguments.boolean_arguments),
                                            std::move(arguments.numerical_arguments));
                        auto step = ModuleProofStep<Kind>(ModuleProgramOutcome::APPLIED, std::move(target));
                        step.rule = rule_variant;
                        result.push_back(std::move(step));
                    }
                },
                rule_variant.get_variant());
        }
    }

    return result;
}

template<tyr::planning::TaskKind Kind>
auto enumerate_sketch_control_steps(const EvaluationContext<Kind>& context, const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
    -> std::vector<ModuleProofStep<Kind>>
{
    auto result = std::vector<ModuleProofStep<Kind>> {};
    const auto source_state = context.get_state();
    const auto module = context.get_module();

    for (const auto& transition : module.get_memory_transitions())
    {
        if (transition.source != context.get_memory_state().get_index())
            continue;

        for (auto rule_variant : tyr::make_view(transition.rules, context.get_repository()))
        {
            tyr::visit(
                [&](auto rule)
                {
                    using RuleViewT = std::decay_t<decltype(rule)>;
                    if constexpr (std::same_as<RuleViewT, RuleView<SketchTag>>)
                    {
                        if (!has_current_source(rule, context))
                            return;

                        auto eval_context = context;
                        if (rule.get_effects().empty() && conditions_are_compatible(rule, eval_context))
                        {
                            auto target = context;
                            target.set_memory_state(rule.get_target());
                            auto step = ModuleProofStep<Kind>(ModuleProgramOutcome::APPLIED, std::move(target));
                            step.rule = rule_variant;
                            result.push_back(std::move(step));
                            return;
                        }

                        eval_context = context;
                        const auto successor = select_sketch_successor(rule, eval_context, successors);
                        if (!successor)
                            return;

                        auto target = context;
                        target.set_state(successor->node.get_state());
                        target.set_memory_state(rule.get_target());
                        auto step = ModuleProofStep<Kind>(ModuleProgramOutcome::APPLIED, std::move(target));
                        step.rule = rule_variant;
                        step.plan_suffix.push_back(*successor);
                        const auto cost = source_state.get_index() == successor->node.get_state().get_index() ? tyr::float_t(0) : tyr::float_t(1);
                        step.state_transition = datasets::StateGraphEdgeLabel { successor->label, cost };
                        result.push_back(std::move(step));
                    }
                },
                rule_variant.get_variant());
        }
    }

    return result;
}

template<tyr::planning::TaskKind Kind>
auto enumerate_control_steps(const runir::datasets::TaskSearchContext<Kind>& search_context,
                             const EvaluationContext<Kind>& context,
                             const ModuleExecutionOptions<Kind>& options) -> std::vector<ModuleProofStep<Kind>>
{
    if (auto result = enumerate_immediate_control_steps(search_context, context); !result.empty())
        return result;

    const auto node = search_context.successor_generator->get_node(context.get_state().get_index());
    const auto successors = search_context.successor_generator->get_labeled_successor_nodes(node);
    if (auto result = enumerate_sketch_control_steps(context, successors); !result.empty())
        return result;

    auto transition_context = context;
    const auto search_result = find_module_program_transition_node(search_context, transition_context, options);
    if (search_result.status == tyr::planning::SearchStatus::SOLVED && search_result.goal_node)
    {
        auto match_context = context;
        if (const auto rule = find_matching_sketch_rule(match_context, search_result.goal_node->get_state()))
        {
            auto rule_variant = find_matching_sketch_rule_variant(match_context, search_result.goal_node->get_state());
            auto target = context;
            target.set_state(search_result.goal_node->get_state());
            target.set_memory_state(rule->get_target());
            auto step = ModuleProofStep<Kind>(ModuleProgramOutcome::APPLIED, std::move(target));
            step.rule = rule_variant;
            append_plan_suffix(step.plan_suffix, search_result);
            if (!step.plan_suffix.empty())
            {
                const auto cost = search_result.plan ? static_cast<tyr::float_t>(search_result.plan->get_length()) : tyr::float_t(1);
                step.state_transition = datasets::StateGraphEdgeLabel { step.plan_suffix.front().label, cost };
            }
            return std::vector<ModuleProofStep<Kind>> { std::move(step) };
        }
    }

    if (search_result.status == tyr::planning::SearchStatus::OUT_OF_TIME)
        return std::vector<ModuleProofStep<Kind>> { ModuleProofStep<Kind>(ModuleProgramOutcome::OUT_OF_TIME, context) };
    if (search_result.status == tyr::planning::SearchStatus::OUT_OF_STATES || search_result.status == tyr::planning::SearchStatus::OUT_OF_MEMORY)
        return std::vector<ModuleProofStep<Kind>> { ModuleProofStep<Kind>(ModuleProgramOutcome::OUT_OF_STATES, context) };

    auto eval_context = context;
    if (eval_context.restore_caller())
        return std::vector<ModuleProofStep<Kind>> { ModuleProofStep<Kind>(ModuleProgramOutcome::RESTORED_CALLER, std::move(eval_context)) };

    return std::vector<ModuleProofStep<Kind>> { ModuleProofStep<Kind>(ModuleProgramOutcome::NO_APPLICABLE_ACTION, context) };
}

template<tyr::planning::TaskKind Kind>
auto execute_solution_path(const runir::datasets::TaskSearchContext<Kind>& search_context,
                           runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                           ModuleProgramView program,
                           const ModuleExecutionOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    auto execution_state = ModuleProgramExecutionState<Kind>(search_context, program);
    auto& context = execution_state.get_context();
    const auto& initial_node = execution_state.get_initial_node();
    auto proof = ModuleProgramProofBuilder<Kind>(search_context, initial_node, std::move(context_owner));
    auto plan_steps = tyr::planning::LabeledNodeList<Kind> {};

    auto current_vertex = proof.get_or_create_vertex(context, ExternalMemoryState(context.get_memory_state()), true, true, false).first;

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
            {
                proof.set_final_state(context.get_state());
                proof.set_plan(tyr::planning::Plan<Kind>(initial_node, std::move(plan_steps)));
                return proof.finish(ModuleProgramProofStatus::SUCCESS);
            }
        }

        const auto source_vertex = current_vertex;
        const auto step_result = execute_next_control_step(search_context, context, options);

        if (step_result.status == ModuleProgramOutcome::APPLIED || step_result.status == ModuleProgramOutcome::RESTORED_CALLER)
        {
            plan_steps.insert(plan_steps.end(), step_result.plan_suffix.begin(), step_result.plan_suffix.end());
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

template<tyr::planning::TaskKind Kind>
auto find_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                   runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                   ModuleProgramView program,
                   const ModuleExecutionOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    return execute_solution_path(search_context, std::move(context_owner), program, options);
}

template<tyr::planning::TaskKind Kind>
auto prove_solution(const runir::datasets::TaskSearchContext<Kind>& search_context,
                    runir::datasets::TaskSearchContextPtr<Kind> context_owner,
                    ModuleProgramView program,
                    const ModuleExecutionOptions<Kind>& options) -> ModuleProgramProofResults<Kind>
{
    auto execution_state = ModuleProgramExecutionState<Kind>(search_context, program);
    auto proof = ModuleProgramProofBuilder<Kind>(search_context, execution_state.get_initial_node(), std::move(context_owner));
    auto open = std::vector<std::pair<EvaluationContext<Kind>, graphs::VertexIndex>> {};
    auto failed = false;

    auto& initial_context = execution_state.get_context();
    const auto initial_vertex = proof.get_or_create_vertex(initial_context, ExternalMemoryState(initial_context.get_memory_state()), true, true, false).first;
    open.emplace_back(initial_context, initial_vertex);

    auto add_terminal = [&](graphs::VertexIndex source, const EvaluationContext<Kind>& context, MemoryStateVariant memory_state, ModuleProgramOutcome status)
    {
        const auto [target, created] = proof.get_or_create_vertex(context, std::move(memory_state), false, false, true);
        static_cast<void>(created);
        const auto edge = proof.add_edge(source, target, std::nullopt);
        proof.add_deadend_transition(edge);
        proof.add_open_state(target);
        failed = true;
        static_cast<void>(status);
    };

    auto add_successor = [&](graphs::VertexIndex source,
                             const ModuleProofStep<Kind>& step,
                             MemoryStateVariant memory_state,
                             bool internal_phase)
    {
        const auto [target, created] = proof.get_or_create_vertex(step.context, std::move(memory_state), false, true, false);
        proof.add_edge(source, target, step.state_transition, step.rule);
        if (created && !proof.is_goal(step.context.get_state()))
            open.emplace_back(step.context, target);
        static_cast<void>(internal_phase);
    };

    while (!open.empty())
    {
        auto [context, source_vertex] = std::move(open.back());
        open.pop_back();

        if (proof.is_goal(context.get_state()))
            continue;

        const auto load_steps = enumerate_load_steps(context);
        if (!load_steps.empty())
        {
            for (const auto& step : load_steps)
            {
                if (step.status == ModuleProgramOutcome::APPLIED)
                    add_successor(source_vertex, step, InternalMemoryState(step.context.get_memory_state()), true);
                else
                    add_terminal(source_vertex, step.context, InternalMemoryState(step.context.get_memory_state()), step.status);
            }
            continue;
        }

        const auto control_steps = enumerate_control_steps(search_context, context, options);
        for (const auto& step : control_steps)
        {
            if (step.status == ModuleProgramOutcome::APPLIED || step.status == ModuleProgramOutcome::RESTORED_CALLER)
                add_successor(source_vertex, step, ExternalMemoryState(step.context.get_memory_state()), false);
            else if (step.status == ModuleProgramOutcome::OUT_OF_TIME)
                return proof.finish(ModuleProgramProofStatus::OUT_OF_TIME);
            else if (step.status == ModuleProgramOutcome::OUT_OF_STATES)
                return proof.finish(ModuleProgramProofStatus::OUT_OF_STATES);
            else
                add_terminal(source_vertex, step.context, ExternalMemoryState(step.context.get_memory_state()), step.status);
        }
    }

    return proof.finish(failed ? ModuleProgramProofStatus::FAILURE : ModuleProgramProofStatus::SUCCESS);
}
}  // namespace runir::kr::ps::ext::detail

#endif
