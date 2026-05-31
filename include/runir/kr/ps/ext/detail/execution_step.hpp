#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/datasets/task_class.hpp"
#include "runir/kr/ps/ext/detail/plan_trace.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/detail/execution.hpp"
#include "runir/kr/ps/ext/module_program_executor.hpp"

#include <optional>
#include <tyr/planning/node.hpp>
#include <vector>

namespace runir::kr::ps::ext::detail
{

enum class ModuleProgramOutcome
{
    SUCCESS,
    APPLIED,
    RESTORED_CALLER,
    FAILURE,
    NO_APPLICABLE_ACTION,
    MALFORMED_CALL,
    SEARCH_FAILURE,
    OUT_OF_TIME,
    OUT_OF_STATES,
    CYCLE,
};

template<tyr::planning::TaskKind Kind>
struct ModuleExecutionOptions
{
    tyr::planning::brfs::Options<Kind> brfs_options;
    tyr::planning::iw::Options<Kind> iw_options;
    ygg::uint_t max_arity = 0;
};

template<tyr::planning::TaskKind Kind>
struct ModuleStepResult
{
    ModuleProgramOutcome status = ModuleProgramOutcome::FAILURE;
    std::optional<datasets::StateGraphEdgeLabel> state_transition = std::nullopt;
    std::optional<RuleVariantView> rule = std::nullopt;
    tyr::planning::LabeledNodeList<Kind> plan_suffix;
};

template<tyr::planning::TaskKind Kind>
auto find_module_program_transition_node(const runir::datasets::TaskSearchContext<Kind>& search_context,
                                         EvaluationContext<Kind>& context,
                                         const ModuleExecutionOptions<Kind>& options) -> tyr::planning::SearchResult<Kind>;

template<tyr::planning::TaskKind Kind>
ModuleExecutionOptions<Kind> execution_options(const ModuleProgramSearchOptions<Kind>& options)
{
    auto result = ModuleExecutionOptions<Kind> {};
    result.brfs_options = options.brfs_options;
    result.iw_options = options.iw_options;
    result.max_arity = options.max_arity;
    return result;
}

inline ModuleProgramProofStatus translate_proof_status(ModuleProgramOutcome status)
{
    switch (status)
    {
        case ModuleProgramOutcome::SUCCESS:
            return ModuleProgramProofStatus::SUCCESS;
        case ModuleProgramOutcome::APPLIED:
        case ModuleProgramOutcome::RESTORED_CALLER:
            return ModuleProgramProofStatus::FAILURE;
        case ModuleProgramOutcome::OUT_OF_TIME:
            return ModuleProgramProofStatus::OUT_OF_TIME;
        case ModuleProgramOutcome::OUT_OF_STATES:
            return ModuleProgramProofStatus::OUT_OF_STATES;
        case ModuleProgramOutcome::CYCLE:
            return ModuleProgramProofStatus::FAILURE;
        case ModuleProgramOutcome::FAILURE:
        case ModuleProgramOutcome::NO_APPLICABLE_ACTION:
        case ModuleProgramOutcome::MALFORMED_CALL:
        case ModuleProgramOutcome::SEARCH_FAILURE:
            return ModuleProgramProofStatus::FAILURE;
    }

    return ModuleProgramProofStatus::FAILURE;
}

template<tyr::planning::TaskKind Kind>
ModuleStepResult<Kind> make_applied_step(const tyr::planning::StateView<Kind>& source_state,
                                         const tyr::planning::StateView<Kind>& target_state,
                                         const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    auto result = ModuleStepResult<Kind> {};
    result.status = ModuleProgramOutcome::APPLIED;
    const auto cost = source_state.get_index() == target_state.get_index() ? ygg::float_t(0) : ygg::float_t(1);
    append_single_step_plan(result.plan_suffix, source_state, target_state, successors);
    if (!result.plan_suffix.empty())
        result.state_transition = datasets::StateGraphEdgeLabel { result.plan_suffix.front().label, cost };
    return result;
}

template<tyr::planning::TaskKind Kind>
ModuleStepResult<Kind> make_terminal_step(ModuleProgramOutcome status)
{
    auto result = ModuleStepResult<Kind> {};
    result.status = status;
    return result;
}

template<tyr::planning::TaskKind Kind>
ModuleStepResult<Kind> execute_next_control_step(const runir::datasets::TaskSearchContext<Kind>& search_context,
                                                 EvaluationContext<Kind>& context,
                                                 const ModuleExecutionOptions<Kind>& options)
{
    const auto source_state = context.get_state();
    const auto node = search_context.successor_generator->get_node(context.get_state().get_index());
    const auto successors = search_context.successor_generator->get_labeled_successor_nodes(node);
    const auto immediate_rule = find_applicable_immediate_external_rule(context);
    const auto rule_status = execute_next_immediate_external_rule(context, successors);

    if (rule_status == RuleExecutionStatus::APPLIED)
    {
        auto result = make_applied_step(source_state, context.get_state(), successors);
        result.rule = immediate_rule;
        return result;
    }

    if (rule_status == RuleExecutionStatus::NO_APPLICABLE_ACTION && context.restore_caller())
        return make_terminal_step<Kind>(ModuleProgramOutcome::RESTORED_CALLER);

    if (rule_status == RuleExecutionStatus::NO_APPLICABLE_RULE)
    {
        const auto sketch_rule = find_applicable_sketch_rule(context, successors);
        const auto sketch_status = execute_next_sketch_rule(context, successors);
        if (sketch_status == RuleExecutionStatus::APPLIED)
        {
            auto result = make_applied_step(source_state, context.get_state(), successors);
            result.rule = sketch_rule;
            return result;
        }

        const auto search_result = find_module_program_transition_node(search_context, context, options);
        if (search_result.status == tyr::planning::SearchStatus::SOLVED && search_result.goal_node)
        {
            if (const auto rule = find_matching_sketch_rule(context, search_result.goal_node->get_state()))
            {
                auto rule_variant = find_matching_sketch_rule_variant(context, search_result.goal_node->get_state());
                context.set_state(search_result.goal_node->get_state());
                context.set_memory_state(rule->get_target());

                auto result = ModuleStepResult<Kind> {};
                result.status = ModuleProgramOutcome::APPLIED;
                const auto cost = search_result.plan ? static_cast<ygg::float_t>(search_result.plan->get_length()) : ygg::float_t(1);
                result.rule = rule_variant;
                append_plan_suffix(result.plan_suffix, search_result);
                if (!result.plan_suffix.empty())
                    result.state_transition = datasets::StateGraphEdgeLabel { result.plan_suffix.front().label, cost };
                return result;
            }
        }
        else if (search_result.status == tyr::planning::SearchStatus::OUT_OF_TIME)
        {
            return make_terminal_step<Kind>(ModuleProgramOutcome::OUT_OF_TIME);
        }
        else if (search_result.status == tyr::planning::SearchStatus::OUT_OF_STATES || search_result.status == tyr::planning::SearchStatus::OUT_OF_MEMORY)
        {
            return make_terminal_step<Kind>(ModuleProgramOutcome::OUT_OF_STATES);
        }

        if (context.restore_caller())
            return make_terminal_step<Kind>(ModuleProgramOutcome::RESTORED_CALLER);
    }

    if (rule_status == RuleExecutionStatus::NO_APPLICABLE_ACTION)
        return make_terminal_step<Kind>(ModuleProgramOutcome::NO_APPLICABLE_ACTION);
    if (rule_status == RuleExecutionStatus::MALFORMED_CALL)
        return make_terminal_step<Kind>(ModuleProgramOutcome::MALFORMED_CALL);

    return make_terminal_step<Kind>(ModuleProgramOutcome::FAILURE);
}

}  // namespace runir::kr::ps::ext::detail

#endif
