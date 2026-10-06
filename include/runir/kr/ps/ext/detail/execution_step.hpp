#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"

#include <concepts>
#include <cstddef>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <tyr/planning/node.hpp>
#include <utility>
#include <variant>
#include <yggdrasil/containers/unique_object_pool.hpp>
#include <yggdrasil/core/concepts.hpp>

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
template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> Storage = InternedExecutionStorage<Kind>>
struct ProgramStep
{
private:
    runir::kr::TaskContextPtr<Kind> m_task_context;

public:
    using StoredState = typename Storage::StoredState;
    ProgramOutcome status;
    StoredState target;
    std::optional<datasets::StateGraphEdgeLabel> state_transition = std::nullopt;
    std::optional<RuleVariantView> rule = std::nullopt;
    std::optional<tyr::planning::PackedLabeledNode<Kind>> planning_successor = std::nullopt;

    ProgramStep(ProgramOutcome status_, StoredState target_, runir::kr::TaskContextPtr<Kind> task_context) :
        m_task_context(std::move(task_context)),
        status(status_),
        target(std::move(target_))
    {
    }

    std::string_view get_status_name() const { return to_string(status); }
    StoredState get_target() const noexcept { return target; }
    const auto& get_state_transition() const noexcept { return state_transition; }
    const auto& get_rule() const noexcept { return rule; }
};

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> Storage>
ProgramStep<Kind, Storage> make_step(ProgramOutcome status, typename Storage::StoredState state, const runir::kr::TaskContextPtr<Kind>& task_context)
{
    return ProgramStep<Kind, Storage>(status, std::move(state), task_context);
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> Storage>
ProgramStep<Kind, Storage> applied(typename Storage::StoredState state, RuleVariantView rule, const runir::kr::TaskContextPtr<Kind>& task_context)
{
    auto step = make_step<Kind, Storage>(ProgramOutcome::APPLIED, std::move(state), task_context);
    step.rule = rule;
    return step;
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> Storage, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
auto planning_step(Storage& storage,
                   S state,
                   const tyr::planning::LabeledNode<Kind, PS>& successor,
                   RuleVariantView rule,
                   MemoryStateView memory_state,
                   const runir::kr::TaskContextPtr<Kind>& task_context)
{
    const auto module_ = state.get_module_state();
    auto target =
        storage.store(successor.node.get_state(), module_.get_module(), memory_state, module_.get_registers(), module_.get_arguments(), state.get_call_stack());
    auto step = applied<Kind, Storage>(std::move(target), rule, task_context);
    if constexpr (requires { successor.pack(); })
        step.planning_successor = successor.pack();
    step.state_transition = datasets::StateGraphEdgeLabel { successor.label, ygg::float_t(1) };
    return step;
}

/// Retains candidate bindings while child evaluations clear their denotation caches.
/// The expander owns the binding pool and must outlive the choice in every memorization mode.
template<runir::kr::dl::CategoryTag Category>
struct Choice
{
private:
    ygg::UniqueObjectPoolPtr<runir::kr::dl::semantics::DenotationElementViewList<Category>> m_bindings;

public:
    RuleVariantView rule;
    size_t position = 0;

    template<ygg::InputRangeOf<runir::kr::dl::semantics::DenotationElementView<Category>> Denotation>
    Choice(RuleVariantView rule_, Denotation denotation, ygg::UniqueObjectPool<runir::kr::dl::semantics::DenotationElementViewList<Category>>& pool) :
        m_bindings(pool.get_or_allocate()),
        rule(rule_)
    {
        m_bindings->clear();
        for (const auto binding : denotation)
            m_bindings->push_back(binding);
    }
    auto& bindings() noexcept { return *m_bindings; }
    const auto& bindings() const noexcept { return *m_bindings; }
    bool exhausted() const noexcept { return position == bindings().size(); }
    const auto& current() const { return bindings().at(position); }
    void advance() noexcept { ++position; }
    bool has_alternatives() const noexcept { return bindings().size() > 1; }
    size_t count() const noexcept { return bindings().size(); }
};

}  // namespace runir::kr::ps::ext::detail

#endif
