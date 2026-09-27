#ifndef RUNIR_KR_PS_ICP_DETAIL_EXECUTION_STEP_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_EXECUTION_STEP_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/ps/icp/execution_view.hpp"
#include "runir/kr/ps/icp/program_executor_data.hpp"
#include "runir/kr/ps/icp/rule_variant_view.hpp"

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <tyr/planning/node.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace runir::kr::ps::icp::detail
{

/// Local rule outcomes; whole-search completion and limits use ProgramProofStatus.
enum class ProgramOutcome
{
    APPLIED,
    FAILURE,
    NO_APPLICABLE_ACTION,
};

constexpr std::string_view to_string(ProgramOutcome outcome)
{
    switch (outcome)
    {
        case ProgramOutcome::APPLIED:
            return "applied";
        case ProgramOutcome::FAILURE:
            return "failure";
        case ProgramOutcome::NO_APPLICABLE_ACTION:
            return "no_applicable_action";
    }
    throw std::invalid_argument("invalid ProgramOutcome");
}

/// One rule application, including local failures whose target remains the source state.
/// Search completion and resource limits are reported separately from these steps.
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
    std::optional<tyr::planning::PackedLabeledNode<Kind>> planning_successor = std::nullopt;

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

}  // namespace runir::kr::ps::icp::detail

#endif
