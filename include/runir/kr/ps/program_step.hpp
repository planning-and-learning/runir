#ifndef RUNIR_KR_PS_PROGRAM_STEP_HPP_
#define RUNIR_KR_PS_PROGRAM_STEP_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/declarations.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tyr/planning/node.hpp>
#include <utility>

namespace runir::kr::ps
{

/// Local rule and caller-return outcomes; whole-search completion and limits use ProofStatus.
/// icp has no calls: it emits neither RESTORED_CALLER nor MALFORMED_CALL.
enum class ProgramOutcome : std::uint8_t
{
    APPLIED,
    RESTORED_CALLER,
    FAILURE,
    NO_APPLICABLE_ACTION,
    MALFORMED_CALL,
};

constexpr std::string_view to_string(ProgramOutcome outcome)
{
    switch (outcome)
    {
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
    }
    throw std::invalid_argument("invalid ProgramOutcome");
}

/// One rule application or caller return, including local failures whose target remains the source state.
/// Search completion and resource limits are reported separately from these steps.
/// `Target` is how the expander stores program states: interned views by default.
template<ProgramFamilyTag Family, tyr::TaskKind Kind, typename Target = ProgramStateView<Family, Kind>>
struct ProgramStep
{
    /// Keeps the task's repositories alive while the step refers to them.
    runir::kr::TaskContextPtr<Kind> task_context;
    ProgramOutcome status;
    Target target;
    std::optional<datasets::StateGraphEdgeLabel> state_transition = std::nullopt;
    std::optional<RuleView<Family>> rule = std::nullopt;
    std::optional<tyr::planning::PackedLabeledNode<Kind>> planning_successor = std::nullopt;

    ProgramStep(ProgramOutcome status_, Target target_, runir::kr::TaskContextPtr<Kind> task_context_) :
        task_context(std::move(task_context_)),
        status(status_),
        target(std::move(target_))
    {
    }

    [[nodiscard]] std::string_view get_status_name() const { return to_string(status); }
    [[nodiscard]] Target get_target() const noexcept { return target; }
    [[nodiscard]] const auto& get_state_transition() const noexcept { return state_transition; }
    [[nodiscard]] const auto& get_rule() const noexcept { return rule; }
};

}  // namespace runir::kr::ps

#endif
