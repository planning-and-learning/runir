#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tyr/planning/node.hpp>
#include <utility>
#include <variant>

namespace runir::kr::ps::ext::detail
{

/// Local rule and caller-return outcomes; whole-search completion and limits use ProgramProofStatus.
enum class ProgramOutcome
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

template<runir::kr::dl::CategoryTag Category>
struct Choice
{
    using Denotation = runir::kr::dl::semantics::DenotationView<Category>;
    using Cursor = decltype(std::declval<Denotation>().begin());

    RuleVariantView rule;
    Denotation denotation;
    Cursor cursor;

    Choice(RuleVariantView rule_, Denotation denotation_) noexcept : rule(rule_), denotation(denotation_), cursor(denotation.begin()) {}

    bool exhausted() const noexcept { return cursor == denotation.end(); }
    auto current() const noexcept { return *cursor; }
    void advance() noexcept { ++cursor; }

    bool has_alternatives() const noexcept
    {
        auto first = denotation.begin();
        return first != denotation.end() && ++first != denotation.end();
    }

    size_t count() const noexcept
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return denotation.get().count();
        else
            return denotation.count();
    }
};

template<tyr::TaskKind Kind>
struct ChoiceFrame
{
    ProgramStateView<Kind> state;
    std::variant<Choice<runir::kr::dl::ConceptTag>, Choice<runir::kr::dl::RoleTag>> choice;
};

}  // namespace runir::kr::ps::ext::detail

#endif
