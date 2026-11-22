#ifndef RUNIR_KR_PS_EXT_EXECUTION_STORAGE_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_STORAGE_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/interning.hpp"
#include "runir/kr/dl/semantics/register_values_view_concepts.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/execution_repository.hpp"
#include "runir/kr/ps/ext/execution_builder.hpp"
#include "runir/kr/ps/ext/execution_data.hpp"
#include "runir/kr/ps/ext/execution_view.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/task_context.hpp"
#include "tyr/planning/successor_generator_concepts.hpp"

#include <concepts>
#include <optional>
#include <tyr/formalism/declarations.hpp>
#include <tyr/planning/ground/successor_generator.hpp>
#include <tyr/planning/lifted/successor_generator.hpp>
#include <tyr/planning/node.hpp>
#include <urd/repr/slots/slots.hpp>
#include <urd/store.hpp>
#include <utility>
#include <yggdrasil/formalism/row_view.hpp>

namespace runir::kr::ps::ext
{

template<typename Storage>
using StorageStoredState = typename Storage::StoredState;

template<typename Storage>
using StorageStateView = typename Storage::StateView;

/// A program state's planning state as its storage holds it, read: an interned program state's (registered, packed)
/// unpacked; a transient one's (always built: TransientExecutionStorage::store) its builder's view.
template<tyr::TaskKind Kind>
tyr::planning::StateView<Kind> planning_state(ProgramStateView<Kind> state)
{ return state.get_state().unpack(); }

template<tyr::TaskKind Kind>
tyr::planning::StateBuilderView<Kind> planning_state(const ProgramStateBuilderView<Kind>& state)
{ return std::get<tyr::planning::StateBuilderView<Kind>>(state.get_state()); }

/// A program state's call stack: its top frame, interned in either storage (frames are never built).
template<tyr::TaskKind Kind>
std::optional<CallStackView> call_stack(ProgramStateView<Kind> state)
{ return state.get_call_stack(); }

template<tyr::TaskKind Kind>
std::optional<CallStackView> call_stack(const ProgramStateBuilderView<Kind>& state)
{
    const auto frame = state.get_call_stack();
    return frame.has_value() ? std::optional(urd::interned_view(*frame)) : std::nullopt;
}

/// Shared execution operations; each storage chooses its own owned handles and borrowed views, and the planning states
/// its successors are generated from (PlanningState).
template<typename Storage, typename Kind>
concept ExecutionStorageConcept = requires(Storage& storage,
                                           const Storage& const_storage,
                                           tyr::planning::StateView<Kind> initial,
                                           const StorageStoredState<Storage>& stored,
                                           StorageStateView<Storage> state,
                                           ModuleView module_,
                                           MemoryStateView memory,
                                           ModuleStateView module_state,
                                           urd::data<runir::kr::dl::semantics::RegisterValues>& registers,
                                           runir::kr::dl::semantics::RegisterValuesView saved_registers,
                                           runir::kr::dl::semantics::CallArgumentsView arguments,
                                           std::optional<CallStackView> caller,
                                           const typename Storage::PlanningState& planning_state,
                                           tyr::formalism::ActionRowView binding,
                                           tyr::planning::ActionBindingView<Kind> borrowed_binding) {
    requires tyr::TaskKind<Kind>;
    requires std::copyable<StorageStoredState<Storage>>;
    requires urd::ViewOf<StorageStateView<Storage>, ProgramState<Kind>>;
    { storage.initial_state(initial) } -> std::same_as<StorageStoredState<Storage>>;
    { storage.start_state(std::declval<ProgramStateView<Kind>>()) } -> std::same_as<StorageStoredState<Storage>>;
    { const_storage.view(stored) } -> std::same_as<StorageStateView<Storage>>;
    { storage.retain(state) } -> std::same_as<StorageStoredState<Storage>>;
    { storage.registers(registers) } -> urd::ReadsAs<runir::kr::dl::semantics::RegisterValues>;
    { storage.store(planning_state, module_, memory, storage.registers(registers), arguments, caller) } -> std::same_as<StorageStoredState<Storage>>;
    { storage.store(initial, module_, memory, saved_registers, arguments, caller) } -> std::same_as<StorageStoredState<Storage>>;
    { storage.store(initial, module_state, caller) } -> std::same_as<StorageStoredState<Storage>>;
    { storage.save_caller(state, memory) } -> std::same_as<CallStackView>;
    { storage.materialize(state) } -> std::same_as<ProgramStateView<Kind>>;
    { storage.successor(planning_state, binding) } -> std::same_as<tyr::planning::Node<Kind, typename Storage::PlanningState>>;
    { storage.successor(planning_state, borrowed_binding) } -> std::same_as<tyr::planning::Node<Kind, typename Storage::PlanningState>>;
};

/// Emission consumes this storage's source view; matching may inspect other program-state views.
template<typename State, typename Storage>
concept ExecutionStateViewConcept = std::same_as<State, StorageStateView<Storage>>;

/// Repository-backed construction used when the full explored graph is retained.
template<tyr::TaskKind Kind>
class InternedExecutionStorage
{
    runir::kr::TaskContextPtr<Kind> m_context;
    ProgramView m_program;

public:
    using StoredState = ProgramStateView<Kind>;
    using StateView = ProgramStateView<Kind>;
    using PlanningState = tyr::planning::StateView<Kind>;

    InternedExecutionStorage(runir::kr::TaskContextPtr<Kind> context, ProgramView program) : m_context(std::move(context)), m_program(program) {}

    runir::kr::dl::semantics::RegisterValuesView registers(urd::data<runir::kr::dl::semantics::RegisterValues>& data)
    { return m_context->dl_denotation_repository->insert(data).first; }

    [[nodiscard]] StateView view(StoredState state) const noexcept { return state; }

    [[nodiscard]] StoredState retain(StateView state) const noexcept { return state; }

    /// A program state in this activation: its planning state (registered) and call stack, the activation shared.
    StoredState store(const tyr::planning::StateView<Kind>& planning_state, ModuleStateView module_state, std::optional<CallStackView> caller)
    {
        auto& builder = m_context->execution_builder;
        auto program_state = builder.template checkout<ProgramState<Kind>>();
        program_state->state = planning_state.pack().index();
        urd::store(program_state->module_state, module_state);
        if (caller.has_value())
            urd::store(program_state->call_stack, *caller);
        return m_context->execution_repository->insert(*program_state, builder).first;
    }

    /// A program state in a new activation of these parts (each interned).
    StoredState store(const tyr::planning::StateView<Kind>& planning_state,
                      ModuleView module_,
                      MemoryStateView memory_state,
                      runir::kr::dl::semantics::RegisterValuesView registers,
                      runir::kr::dl::semantics::CallArgumentsView arguments,
                      std::optional<CallStackView> caller)
    { return store(planning_state, activation(module_, memory_state, registers, arguments), caller); }

    StoredState initial_state(const tyr::planning::StateView<Kind>& state)
    {
        const auto entry = m_program.get_entry_mod();
        auto empty = m_context->dl_builder.template scratch<runir::kr::dl::semantics::RegisterValues>();
        empty->concept_values.resize(entry.template get_registers<runir::kr::dl::ConceptTag>().size());
        empty->role_values.resize(entry.template get_registers<runir::kr::dl::RoleTag>().size());
        auto arguments = m_context->dl_builder.template scratch<runir::kr::dl::semantics::CallArguments>();
        return store(state,
                     entry,
                     entry.get_entry_memory_state(),
                     registers(*empty),
                     m_context->dl_denotation_repository->insert(*arguments).first,
                     std::optional<CallStackView>());
    }

    /// Continue from an interned program state of this task and program.
    [[nodiscard]] StoredState start_state(ProgramStateView<Kind> state) const noexcept { return state; }

    ProgramStateView<Kind> materialize(ProgramStateView<Kind> state) { return state; }

    /// The caller's frame: its activation, returning to `return_state`, and its own caller.
    CallStackView save_caller(ProgramStateView<Kind> state, MemoryStateView return_state)
    {
        const auto current = state.get_module_state();
        auto& builder = m_context->control_builder;
        auto frame = builder.template checkout<CallStack>();
        urd::store(frame->module_state, activation(current.get_mod(), return_state, current.get_registers(), current.get_arguments()));
        if (const auto caller = state.get_call_stack(); caller.has_value())
            urd::store(frame->caller, *caller);
        return m_context->control_repository->insert(*frame, builder).first;
    }

    /// From a planning state of either kind (registered, or being built).
    template<tyr::planning::StateViewConcept<Kind> S, ygg::RowViewConcept<tyr::formalism::Action<tyr::LiftedTag>> Binding>
    tyr::planning::Node<Kind> successor(const S& state, Binding binding)
    {
        auto& search = *m_context->search_context;
        return search.successor_generator->get_successor_node(tyr::planning::Node<Kind, S>(state, 0),
                                                              binding,
                                                              *search.state_repository,
                                                              *search.axiom_evaluator);
    }

private:
    /// The activation of these parts, interned.
    ModuleStateView activation(ModuleView module_,
                               MemoryStateView memory_state,
                               runir::kr::dl::semantics::RegisterValuesView registers,
                               runir::kr::dl::semantics::CallArgumentsView arguments)
    {
        auto& builder = m_context->control_builder;
        auto activation = builder.template checkout<ModuleState>();
        urd::store(activation->mod, module_);
        urd::store(activation->memory_state, memory_state);
        urd::store(activation->registers, registers);
        urd::store(activation->arguments, arguments);
        return m_context->control_repository->insert(*activation, builder).first;
    }
};

/// Pooled per-state construction used by NONE and CHOICE. Selected output paths and
/// CHOICE sources with Choose obligations are materialized in the task repositories.
/// Call arguments and their final denotations are always interned in the task repository.
/// Caller frames and their saved registers are interned only when a call occurs.
/// Returned transient states and handles must be released before this storage is destroyed.
template<tyr::TaskKind Kind>
class TransientExecutionStorage
{
    urd::builder<tyr::planning::State<Kind>, tyr::planning::StateRepository<Kind>> m_planning;
    /// The pools the program states' built children come from (urd::handle), each declared before what holds its
    /// handles, so that it outlives them: planning states and registers, then activations (module states), then
    /// program states.
    urd::Store<urd::slots<urd::builder<tyr::planning::State<Kind>, tyr::planning::StateRepository<Kind>>>> m_planning_states;
    urd::Store<urd::slots<urd::builder<runir::kr::dl::semantics::RegisterValues, runir::kr::dl::semantics::DenotationRepository>>> m_registers;
    urd::Store<urd::slots<urd::builder<ModuleState, ControlRepository>>> m_module_states;
    urd::Store<urd::slots<urd::builder<ProgramState<Kind>, ExecutionRepository<runir::kr::ExtFamilyTag, Kind>>>> m_program_states;
    /// The frame being saved: its activation is built from the pools above, so it is this storage's own (a context's
    /// builder outlives the storage).
    urd::Store<urd::slots<urd::builder<CallStack, ControlRepository>>> m_frames;
    runir::kr::TaskContextPtr<Kind> m_context;
    ProgramView m_program;

public:
    using StoredState = std::shared_ptr<urd::builder<ProgramState<Kind>, ExecutionRepository<runir::kr::ExtFamilyTag, Kind>>>;
    using StateView = ProgramStateBuilderView<Kind>;
    using PlanningState = tyr::planning::StateBuilderView<Kind>;

    TransientExecutionStorage(runir::kr::TaskContextPtr<Kind> context, ProgramView program) : m_context(std::move(context)), m_program(program) {}

    [[nodiscard]] StateView view(const StoredState& state) const noexcept { return urd::resolve(*state, *m_context->execution_repository); }

    StoredState retain(StateView state) { return store(planning_state(state), state.get_module_state(), call_stack(state)); }

    runir::kr::dl::semantics::RegisterValuesDataView registers(const urd::data<runir::kr::dl::semantics::RegisterValues>& data)
    { return urd::resolve(data, *m_context->dl_denotation_repository); }

    /// A program state in this activation (as read: an interned one shared, a built one copied into a builder of its
    /// own), with this planning state (copied into a builder of its own) and call stack (interned).
    template<tyr::planning::StateViewConcept<Kind> S, urd::ReadsAs<ModuleState> M>
    StoredState store(const S& planning_state, const M& module_state, std::optional<CallStackView> caller)
    {
        auto result = program_state(planning_state, caller);
        urd::visit_view(module_state,
                        [&](const auto& activation)
                        {
                            if constexpr (std::same_as<std::remove_cvref_t<decltype(activation)>, ModuleStateView>)
                                urd::store(result->module_state, activation);
                            else
                                write_activation(result->module_state,
                                                 activation.get_mod(),
                                                 activation.get_memory_state(),
                                                 activation.get_registers(),
                                                 activation.get_arguments());
                        });
        return result;
    }

    /// A program state in a new activation of these parts, each as read (its registers copied into a builder of their
    /// own, its other parts interned).
    template<tyr::planning::StateViewConcept<Kind> S,
             urd::ReadsAs<Module> M,
             urd::ReadsAs<MemoryState<runir::kr::ExtFamilyTag>> Memory,
             urd::ReadsAs<runir::kr::dl::semantics::RegisterValues> R,
             urd::ReadsAs<runir::kr::dl::semantics::CallArguments> A>
    StoredState
    store(const S& planning_state, const M& module_, const Memory& memory_state, const R& registers, const A& arguments, std::optional<CallStackView> caller)
    {
        auto result = program_state(planning_state, caller);
        write_activation(result->module_state, module_, memory_state, registers, arguments);
        return result;
    }

    StoredState initial_state(const tyr::planning::StateView<Kind>& state)
    {
        const auto entry = m_program.get_entry_mod();
        auto empty = m_context->dl_builder.template scratch<runir::kr::dl::semantics::RegisterValues>();
        empty->concept_values.resize(entry.template get_registers<runir::kr::dl::ConceptTag>().size());
        empty->role_values.resize(entry.template get_registers<runir::kr::dl::RoleTag>().size());
        auto arguments = m_context->dl_builder.template scratch<runir::kr::dl::semantics::CallArguments>();
        return store(state,
                     entry,
                     entry.get_entry_memory_state(),
                     registers(*empty),
                     m_context->dl_denotation_repository->insert(*arguments).first,
                     std::optional<CallStackView>());
    }

    /// Continue from an interned program state of this task and program.
    StoredState start_state(ProgramStateView<Kind> state) { return store(planning_state(state), state.get_module_state(), call_stack(state)); }

    /// The program state interned: its builder's children through the repositories that hold them (its planning
    /// state registered in the state repository, its registers in the denotation repository), then itself.
    ProgramStateView<Kind> materialize(ProgramStateBuilderView<Kind> state)
    { return m_context->execution_repository->insert(state, m_context->execution_builder).first; }

    /// The caller's frame, interned: its activation, returning to `return_state`, and its own caller.
    CallStackView save_caller(ProgramStateBuilderView<Kind> state, MemoryStateView return_state)
    {
        auto frame = m_frames.make_shared();
        urd::visit_view(state.get_module_state(),
                        [&](const auto& activation)
                        { write_activation(frame->module_state, activation.get_mod(), return_state, activation.get_registers(), activation.get_arguments()); });
        frame->caller.reset();
        if (const auto caller = call_stack(state); caller.has_value())
            urd::store(frame->caller, *caller);
        return m_context->control_repository->insert(*frame, m_context->control_builder).first;
    }

    /// The returned state borrows scratch storage until the next successor() call.
    /// Consume or copy it before then; the source must not borrow this same scratch storage.
    /// From a planning state of either kind (registered, or being built).
    template<tyr::planning::StateViewConcept<Kind> S, ygg::RowViewConcept<tyr::formalism::Action<tyr::LiftedTag>> Binding>
    tyr::planning::Node<Kind, tyr::planning::StateBuilderView<Kind>> successor(const S& state, Binding binding)
    {
        auto& search = *m_context->search_context;
        return search.successor_generator->get_successor_node(tyr::planning::Node<Kind, S>(state, 0), binding, m_planning, *search.axiom_evaluator);
    }

private:
    /// A program state of this planning state (copied into a builder of its own) and call stack, its activation to write.
    template<tyr::planning::StateViewConcept<Kind> S>
    StoredState program_state(const S& planning_state, std::optional<CallStackView> caller)
    {
        auto result = m_program_states.make_shared();
        // Assign into the destination's existing builders and buffers so released pool slots retain their capacity.
        urd::builder_of(result->state, m_planning_states).assign(planning_state);
        result->call_stack.reset();
        if (caller.has_value())
            urd::store(result->call_stack, *caller);
        return result;
    }

    /// An activation of these parts, each as read, into `target`: a builder of its own, its registers copied into
    /// theirs, its other parts interned (urd::copy_child).
    template<urd::ReadsAs<Module> M,
             urd::ReadsAs<MemoryState<runir::kr::ExtFamilyTag>> Memory,
             urd::ReadsAs<runir::kr::dl::semantics::RegisterValues> R,
             urd::ReadsAs<runir::kr::dl::semantics::CallArguments> A>
    void
    write_activation(urd::child<ModuleState, ControlRepository>& target, const M& module_, const Memory& memory_state, const R& registers, const A& arguments)
    {
        auto& activation = urd::builder_of(target, m_module_states);
        urd::copy_child(activation.mod, module_);
        urd::copy_child(activation.memory_state, memory_state);
        urd::visit_view(registers, [&](const auto& values) { runir::kr::dl::semantics::assign(urd::builder_of(activation.registers, m_registers), values); });
        urd::copy_child(activation.arguments, arguments);
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
