#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/module_program_executor_data.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/task_context.hpp"

#include <optional>
#include <tyr/planning/node.hpp>

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

/// One canonical execution successor plus the rule and optional planning transition that produced it.
template<tyr::planning::TaskKind Kind>
struct ModuleProgramStep
{
private:
    runir::kr::TaskContextPtr<Kind> m_task_context;

public:
    ModuleProgramOutcome status;
    ExecutionStateView<Kind> target;
    std::optional<datasets::StateGraphEdgeLabel> state_transition = std::nullopt;
    std::optional<RuleVariantView> rule = std::nullopt;
    tyr::planning::LabeledNodeList<Kind> plan_suffix;

    ModuleProgramStep(ModuleProgramOutcome status_, ExecutionStateView<Kind> target_, runir::kr::TaskContextPtr<Kind> task_context) :
        m_task_context(std::move(task_context)),
        status(status_),
        target(std::move(target_))
    {
    }

    const char* get_status_name() const noexcept { return outcome_name(status); }
    ExecutionStateView<Kind> get_target() const noexcept { return target; }
    const auto& get_state_transition() const noexcept { return state_transition; }
    const auto& get_rule() const noexcept { return rule; }
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
