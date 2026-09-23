#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <optional>
#include <stdexcept>
#include <string_view>
#include <tyr/planning/node.hpp>
#include <utility>

namespace runir::kr::ps::ext::detail
{

enum class ProgramOutcome
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

constexpr std::string_view to_string(ProgramOutcome outcome)
{
    switch (outcome)
    {
        case ProgramOutcome::SUCCESS:
            return "success";
        case ProgramOutcome::APPLIED:
            return "applied";
        case ProgramOutcome::RESTORED_CALLER:
            return "restored_caller";
        case ProgramOutcome::FAILURE:
            return "failure";
        case ProgramOutcome::NO_APPLICABLE_ACTION:
            return "no_applicable_action";
        case ProgramOutcome::MALFORMED_CALL:
            return "malformed_call";
        case ProgramOutcome::SEARCH_FAILURE:
            return "search_failure";
        case ProgramOutcome::OUT_OF_TIME:
            return "out_of_time";
        case ProgramOutcome::OUT_OF_STATES:
            return "out_of_states";
        case ProgramOutcome::CYCLE:
            return "cycle";
    }
    throw std::invalid_argument("invalid ProgramOutcome");
}

/// One canonical execution successor plus the rule and optional planning transition that produced it.
template<tyr::TaskKind Kind>
struct ProgramStep
{
private:
    runir::kr::TaskContextPtr<Kind> m_task_context;

public:
    ProgramOutcome status;
    ProgramStateView<Kind> target;
    std::optional<datasets::StateGraphEdgeLabel> state_transition = std::nullopt;
    std::optional<RuleVariantView> rule = std::nullopt;
    tyr::planning::PackedLabeledNodeList<Kind> plan_suffix;

    ProgramStep(ProgramOutcome status_, ProgramStateView<Kind> target_, runir::kr::TaskContextPtr<Kind> task_context) :
        m_task_context(std::move(task_context)),
        status(status_),
        target(std::move(target_))
    {
    }

    std::string_view get_status_name() const { return to_string(status); }
    ProgramStateView<Kind> get_target() const noexcept { return target; }
    const auto& get_state_transition() const noexcept { return state_transition; }
    const auto& get_rule() const noexcept { return rule; }
};

inline ProgramProofStatus translate_proof_status(ProgramOutcome status)
{
    switch (status)
    {
        case ProgramOutcome::SUCCESS:
            return ProgramProofStatus::SUCCESS;
        case ProgramOutcome::APPLIED:
        case ProgramOutcome::RESTORED_CALLER:
            return ProgramProofStatus::FAILURE;
        case ProgramOutcome::OUT_OF_TIME:
            return ProgramProofStatus::OUT_OF_TIME;
        case ProgramOutcome::OUT_OF_STATES:
            return ProgramProofStatus::OUT_OF_STATES;
        case ProgramOutcome::CYCLE:
        case ProgramOutcome::FAILURE:
        case ProgramOutcome::NO_APPLICABLE_ACTION:
        case ProgramOutcome::MALFORMED_CALL:
        case ProgramOutcome::SEARCH_FAILURE:
            return ProgramProofStatus::FAILURE;
    }

    return ProgramProofStatus::FAILURE;
}

}  // namespace runir::kr::ps::ext::detail

#endif
