#ifndef RUNIR_KR_PS_EXT_EXECUTION_STORAGE_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_STORAGE_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/task_context.hpp"

#include <concepts>
#include <optional>
#include <utility>
#include <yggdrasil/containers/shared_object_pool.hpp>

namespace runir::kr::ps::ext
{

/// Shared execution operations; each storage chooses its own owned handles and borrowed views.
template<typename Storage, typename Kind>
concept ExecutionStorageConcept = requires(Storage& storage,
                                           const Storage& const_storage,
                                           tyr::planning::StateView<Kind> initial,
                                           const typename Storage::StoredState& stored,
                                           typename Storage::StateView state,
                                           ModuleView module_,
                                           MemoryStateView memory,
                                           ygg::Data<runir::kr::dl::semantics::RegisterValues>& registers,
                                           runir::kr::dl::semantics::RegisterValuesView saved_registers,
                                           runir::kr::dl::semantics::CallArgumentsView arguments,
                                           std::optional<CallStackView<Kind>> caller,
                                           tyr::formalism::planning::ActionBindingView binding) {
    requires tyr::TaskKind<Kind>;
    requires StoredProgramStateConcept<typename Storage::StoredState, Kind>;
    requires ProgramStateViewConcept<typename Storage::StateView, Kind>;
    { storage.initial_state(initial) } -> std::same_as<typename Storage::StoredState>;
    { const_storage.view(stored) } -> std::same_as<typename Storage::StateView>;
    { storage.retain(state) } -> std::same_as<typename Storage::StoredState>;
    { storage.registers(registers) } -> runir::kr::dl::semantics::RegisterValuesViewConcept;
    { storage.store(state.get_state(), module_, memory, storage.registers(registers), arguments, caller) } -> std::same_as<typename Storage::StoredState>;
    { storage.store(state.get_state(), module_, memory, saved_registers, arguments, caller) } -> std::same_as<typename Storage::StoredState>;
    { storage.save_caller(state, memory) } -> std::same_as<CallStackView<Kind>>;
    { storage.materialize(state) } -> std::same_as<ProgramStateView<Kind>>;
    { storage.successor(state.get_state(), binding) } -> std::same_as<tyr::planning::LabeledNode<decltype(state.get_state())>>;
};

/// Repository-backed construction used when the full explored graph is retained.
template<tyr::TaskKind Kind>
class InternedExecutionStorage
{
    runir::kr::TaskContextPtr<Kind> m_context;
    ProgramView m_program;

public:
    using StoredState = ProgramStateView<Kind>;
    using StateView = ProgramStateView<Kind>;

    InternedExecutionStorage(runir::kr::TaskContextPtr<Kind> context, ProgramView program) : m_context(std::move(context)), m_program(program) {}

    auto registers(ygg::Data<runir::kr::dl::semantics::RegisterValues>& data) { return get_or_create(*m_context->dl_denotation_repository, data).first; }

    StateView view(StoredState state) const noexcept { return state; }
    StoredState retain(StateView state) const noexcept { return state; }

    StoredState store(tyr::planning::StateView<Kind> planning_state,
                      ModuleView module_,
                      MemoryStateView memory_state,
                      runir::kr::dl::semantics::RegisterValuesView registers,
                      runir::kr::dl::semantics::CallArgumentsView arguments,
                      std::optional<CallStackView<Kind>> caller)
    {
        auto module_data = ygg::Data<ModuleState<Kind>>(planning_state.get_index(),
                                                        module_.get_index(),
                                                        memory_state.get_index(),
                                                        registers.get_index(),
                                                        arguments.get_index());
        auto data = ygg::Data<ProgramState<Kind>>(m_program.get_index(), get_or_create(*m_context->execution_repository, module_data).first.get_index());
        ygg::set(caller, data.call_stack);
        return get_or_create(*m_context->execution_repository, data).first;
    }

    StoredState initial_state(const tyr::planning::StateView<Kind>& state)
    {
        const auto entry = m_program.get_entry_module();
        auto empty = checkout<runir::kr::dl::semantics::RegisterValues>(m_context->dl_builder);
        empty->concept_values.resize(entry.template get_registers<runir::kr::dl::ConceptTag>().size());
        empty->role_values.resize(entry.template get_registers<runir::kr::dl::RoleTag>().size());
        auto arguments = checkout<runir::kr::dl::semantics::CallArguments>(m_context->dl_builder);
        return store(state,
                     entry,
                     entry.get_entry_memory_state(),
                     registers(*empty),
                     get_or_create(*m_context->dl_denotation_repository, *arguments).first,
                     {});
    }

    ProgramStateView<Kind> materialize(ProgramStateView<Kind> state) { return state; }

    auto save_caller(ProgramStateView<Kind> state, MemoryStateView return_state)
    {
        const auto& module_ = state.get_module_state().get_data();
        auto saved = ygg::Data<CallStack>(module_.module_, return_state.get_index(), module_.registers, module_.arguments, state.get_data().call_stack);
        return get_or_create(*m_context->execution_repository, saved).first;
    }

    tyr::planning::LabeledNode<tyr::planning::StateView<Kind>> successor(const tyr::planning::StateView<Kind>& state,
                                                                         tyr::formalism::planning::ActionBindingView binding)
    {
        auto& search = *m_context->search_context;
        return { binding,
                 search.successor_generator->get_successor_node(tyr::planning::Node<tyr::planning::StateView<Kind>>(state, 0),
                                                                binding,
                                                                *search.state_repository,
                                                                *search.axiom_evaluator) };
    }
};

namespace detail
{

template<runir::kr::dl::semantics::RegisterValuesViewConcept R>
runir::kr::dl::semantics::RegisterValuesView
materialize_register_values(R registers, runir::kr::dl::semantics::Builder& builder, runir::kr::dl::semantics::DenotationRepository& repository)
{
    auto data = runir::kr::dl::semantics::checkout<runir::kr::dl::semantics::RegisterValues>(builder);
    data->concept_values = registers.get_data().concept_values;
    data->role_values = registers.get_data().role_values;
    return runir::kr::dl::semantics::get_or_create(repository, *data).first;
}

template<tyr::TaskKind Kind, ExecutionStorageConcept<Kind> Storage, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
auto planning_step(Storage& storage,
                   S state,
                   const tyr::planning::LabeledNode<PS>& successor,
                   RuleVariantView rule,
                   MemoryStateView memory_state,
                   const runir::kr::TaskContextPtr<Kind>& task_context)
{
    const auto module_ = state.get_module_state();
    auto target =
        storage.store(successor.node.get_state(), module_.get_module(), memory_state, module_.get_registers(), module_.get_arguments(), state.get_call_stack());
    auto step = applied(std::move(target), rule, task_context);
    if constexpr (requires { successor.pack(); })
        step.planning_successor = successor.pack();
    step.state_transition = datasets::StateGraphEdgeLabel { successor.label, ygg::float_t(1) };
    return step;
}

}  // namespace detail

/// Pooled per-state construction used by NONE and CHOICE. Selected output paths and
/// CHOICE sources with Choose obligations are materialized in the task repositories.
/// Call arguments and their final denotations are always interned in the task repository.
/// Caller frames and their saved registers are interned only when a call occurs.
/// Returned transient states and handles must be released before this storage is destroyed.
template<tyr::TaskKind Kind>
class TransientExecutionStorage
{
    ygg::Builder<tyr::planning::State<Kind>> m_planning;
    ygg::SharedObjectPool<ygg::Builder<ProgramState<Kind>>> m_program_states;
    runir::kr::TaskContextPtr<Kind> m_context;
    ProgramView m_program;

public:
    using StoredState = ygg::SharedObjectPoolPtr<ygg::Builder<ProgramState<Kind>>>;
    using StateView = BuilderProgramStateView<Kind>;

    TransientExecutionStorage(runir::kr::TaskContextPtr<Kind> context, ProgramView program) : m_context(std::move(context)), m_program(program) {}

    StateView view(const StoredState& state) const noexcept { return ygg::make_view(*state, *m_context->execution_repository); }
    StoredState retain(StateView state)
    {
        auto result = m_program_states.get_or_allocate();
        *result = state.get_data();
        return result;
    }
    auto registers(const ygg::Data<runir::kr::dl::semantics::RegisterValues>& data)
    {
        return ygg::make_view(data, *m_context->search_context->task->get_repository());
    }
    template<tyr::planning::StateViewConcept<Kind> S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
    StoredState store(const S& planning_state,
                      ModuleView module_,
                      MemoryStateView memory_state,
                      R registers,
                      runir::kr::dl::semantics::CallArgumentsView arguments,
                      std::optional<CallStackView<Kind>> caller)
    {
        auto result = m_program_states.get_or_allocate();
        result->program = m_program.get_index();
        // Assign into the destination's existing buffers so released pool slots retain their capacity.
        auto& target = result->module_state;
        target.state = planning_state.get_state_builder();
        target.module_ = module_.get_index();
        target.memory_state = memory_state.get_index();
        target.registers = registers.get_data();
        target.arguments = arguments.get_index();
        ygg::set(caller, result->call_stack);
        return result;
    }

    StoredState initial_state(const tyr::planning::StateView<Kind>& state)
    {
        const auto entry = m_program.get_entry_module();
        auto empty = checkout<runir::kr::dl::semantics::RegisterValues>(m_context->dl_builder);
        empty->concept_values.resize(entry.template get_registers<runir::kr::dl::ConceptTag>().size());
        empty->role_values.resize(entry.template get_registers<runir::kr::dl::RoleTag>().size());
        auto arguments = checkout<runir::kr::dl::semantics::CallArguments>(m_context->dl_builder);
        return store(state,
                     entry,
                     entry.get_entry_memory_state(),
                     registers(*empty),
                     get_or_create(*m_context->dl_denotation_repository, *arguments).first,
                     {});
    }

    ProgramStateView<Kind> materialize(BuilderProgramStateView<Kind> state)
    {
        auto& search = *m_context->search_context;
        auto builder = search.state_repository->get_state_builder();
        *builder = state.get_state().get_state_builder();
        const auto planning = search.state_repository->register_extended_state(std::move(builder));
        auto& denotations = *m_context->dl_denotation_repository;
        auto& dl_builder = m_context->dl_builder;
        const auto module_ = state.get_module_state();
        auto data = ygg::Data<ModuleState<Kind>>(planning.get_index(),
                                                 module_.get_module().get_index(),
                                                 module_.get_memory_state().get_index(),
                                                 detail::materialize_register_values(module_.get_registers(), dl_builder, denotations).get_index(),
                                                 module_.get_arguments().get_index());
        auto program = ygg::Data<ProgramState<Kind>>(m_program.get_index(),
                                                     get_or_create(*m_context->execution_repository, data).first.get_index(),
                                                     state.get_data().call_stack);
        return get_or_create(*m_context->execution_repository, program).first;
    }

    auto save_caller(BuilderProgramStateView<Kind> state, MemoryStateView return_state)
    {
        const auto module_ = state.get_module_state();
        const auto registers = detail::materialize_register_values(module_.get_registers(), m_context->dl_builder, *m_context->dl_denotation_repository);
        auto saved = ygg::Data<CallStack>(module_.get_module().get_index(),
                                          return_state.get_index(),
                                          registers.get_index(),
                                          module_.get_arguments().get_index(),
                                          state.get_data().call_stack);
        return get_or_create(*m_context->execution_repository, saved).first;
    }

    /// The returned state borrows scratch storage until the next successor() call.
    /// Consume or copy it before then; the source must not borrow this same scratch storage.
    tyr::planning::LabeledNode<tyr::planning::BuilderStateView<Kind>> successor(const tyr::planning::BuilderStateView<Kind>& state,
                                                                                tyr::formalism::planning::ActionBindingView binding)
    {
        auto& search = *m_context->search_context;
        return { binding,
                 search.successor_generator->get_successor_node(tyr::planning::Node<tyr::planning::BuilderStateView<Kind>>(state, 0),
                                                                binding,
                                                                m_planning,
                                                                *search.axiom_evaluator) };
    }
};

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class InternedExecutionStorage<tyr::GroundTag>;
extern template class InternedExecutionStorage<tyr::LiftedTag>;
extern template class TransientExecutionStorage<tyr::GroundTag>;
extern template class TransientExecutionStorage<tyr::LiftedTag>;

#endif

}  // namespace runir::kr::ps::ext

#endif
