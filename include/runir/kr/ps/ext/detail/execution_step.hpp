#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/ext/detail/execution.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/task_context.hpp"

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

inline const char* outcome_name(ModuleProgramOutcome outcome)
{
    switch (outcome)
    {
        case ModuleProgramOutcome::SUCCESS:
            return "success";
        case ModuleProgramOutcome::APPLIED:
            return "applied";
        case ModuleProgramOutcome::RESTORED_CALLER:
            return "restored_caller";
        case ModuleProgramOutcome::FAILURE:
            return "failure";
        case ModuleProgramOutcome::NO_APPLICABLE_ACTION:
            return "no_applicable_action";
        case ModuleProgramOutcome::MALFORMED_CALL:
            return "malformed_call";
        case ModuleProgramOutcome::SEARCH_FAILURE:
            return "search_failure";
        case ModuleProgramOutcome::OUT_OF_TIME:
            return "out_of_time";
        case ModuleProgramOutcome::OUT_OF_STATES:
            return "out_of_states";
        case ModuleProgramOutcome::CYCLE:
            return "cycle";
    }
    return "failure";
}

// The result of applying one rule from a source vertex: the resulting evaluation context (state +
// memory + registers, ready to continue stepping), the planning transition it took (if any), the
// rule responsible, and the planning actions consumed. This is the single currency the
// `SuccessorExpander` produces for universal (all successors) and greedy (first successor) search.
template<tyr::planning::TaskKind Kind>
struct ModuleProgramStep
{
    ModuleProgramOutcome status;
    EvaluationContext<Kind> context;
    std::optional<datasets::StateGraphEdgeLabel> state_transition = std::nullopt;
    std::optional<RuleVariantView> rule = std::nullopt;
    tyr::planning::LabeledNodeList<Kind> plan_suffix;

    ModuleProgramStep(ModuleProgramOutcome status_, EvaluationContext<Kind> context_) : status(status_), context(std::move(context_)) {}

    const char* get_status_name() const noexcept { return outcome_name(status); }
    const EvaluationContext<Kind>& get_target() const noexcept { return context; }

    std::optional<ModuleProgramProofEdgeLabel> get_edge() const
    {
        if (rule || state_transition)
            return ModuleProgramProofEdgeLabel { state_transition, rule };
        return std::nullopt;
    }
};

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

}  // namespace runir::kr::ps::ext::detail

#endif
