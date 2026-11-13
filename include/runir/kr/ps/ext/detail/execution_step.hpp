#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_STEP_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/declarations.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/program_executor_data.hpp"
#include "runir/kr/ps/program_step.hpp"
#include "runir/kr/ps/rule_variant_view.hpp"

#include <concepts>
#include <cstddef>
#include <optional>
#include <ranges>
#include <tyr/planning/node.hpp>
#include <urd/repr/slots/slots.hpp>
#include <urd/store.hpp>
#include <utility>
#include <variant>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> Storage>
ProgramStep<ExtFamilyTag, Kind, StorageStoredState<Storage>>
make_step(ProgramOutcome status, StorageStoredState<Storage> state, const runir::kr::TaskContextPtr<Kind>& task_context)
{ return ProgramStep<ExtFamilyTag, Kind, StorageStoredState<Storage>>(status, std::move(state), task_context); }

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> Storage>
ProgramStep<ExtFamilyTag, Kind, StorageStoredState<Storage>>
applied(StorageStoredState<Storage> state, RuleVariantView rule, const runir::kr::TaskContextPtr<Kind>& task_context)
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
    if constexpr (PackableSuccessor<tyr::planning::LabeledNode<Kind, PS>>)
        step.planning_successor = successor.pack();
    step.state_transition = datasets::StateGraphEdgeLabel { successor.label, static_cast<ygg::float_t>(1) };
    return step;
}

/// Retains candidate bindings while child evaluations clear their denotation caches.
/// The expander owns the binding pool and must outlive the choice in every memorization mode.
template<runir::kr::dl::CategoryTag Category>
struct Choice
{
private:
    urd::unique_slot_ptr<runir::kr::dl::semantics::DenotationElementViewList<Category>> m_bindings;
    RuleVariantView m_rule;
    size_t m_position = 0;

public:
    template<typename Denotation>
        requires std::ranges::input_range<Denotation>
                     && std::same_as<std::ranges::range_value_t<Denotation>, runir::kr::dl::semantics::DenotationElementView<Category>>
    Choice(RuleVariantView rule_, Denotation denotation, urd::Store<urd::slots<runir::kr::dl::semantics::DenotationElementViewList<Category>>>& pool) :
        m_bindings(pool.make_unique()),
        m_rule(rule_)
    {
        m_bindings->clear();
        for (const auto binding : denotation)
            m_bindings->push_back(binding);
    }
    [[nodiscard]] RuleVariantView get_rule() const noexcept { return m_rule; }
    auto& bindings() noexcept { return *m_bindings; }
    [[nodiscard]] const auto& bindings() const noexcept { return *m_bindings; }
    [[nodiscard]] bool exhausted() const noexcept { return m_position == bindings().size(); }
    [[nodiscard]] const auto& current() const { return bindings().at(m_position); }
    void advance() noexcept { ++m_position; }
    [[nodiscard]] bool has_alternatives() const noexcept { return bindings().size() > 1; }
    [[nodiscard]] size_t count() const noexcept { return bindings().size(); }
};

}  // namespace runir::kr::ps::ext::detail

#endif
