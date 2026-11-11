#ifndef RUNIR_KR_PS_EXT_EXECUTION_STORAGE_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_STORAGE_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/dl/semantics/interning.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/execution_repository.hpp"
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

/// Shared execution operations; each storage chooses its own owned handles and borrowed views.
template<typename Storage, typename Kind>
concept ExecutionStorageConcept = requires(Storage& storage,
                                           const Storage& const_storage,
                                           tyr::planning::StateView<Kind> initial,
                                           const StorageStoredState<Storage>& stored,
                                           StorageStateView<Storage> state,
                                           ModuleView module_,
                                           MemoryStateView memory,
                                           urd::data<runir::kr::dl::semantics::RegisterValues>& registers,
                                           runir::kr::dl::semantics::RegisterValuesView saved_registers,
                                           runir::kr::dl::semantics::CallArgumentsView arguments,
                                           std::optional<CallStackView<Kind>> caller,
                                           tyr::formalism::planning::ActionRowView binding,
                                           tyr::planning::BorrowedActionRowView<Kind> borrowed_binding) {
    requires tyr::TaskKind<Kind>;
    requires std::copyable<StorageStoredState<Storage>>;
    requires ProgramStateViewConcept<StorageStateView<Storage>, Kind>;
    { storage.initial_state(initial) } -> std::same_as<StorageStoredState<Storage>>;
    { storage.start_state(std::declval<ProgramStateView<Kind>>()) } -> std::same_as<StorageStoredState<Storage>>;
    { const_storage.view(stored) } -> std::same_as<StorageStateView<Storage>>;
    { storage.retain(state) } -> std::same_as<StorageStoredState<Storage>>;
    { storage.registers(registers) } -> runir::kr::dl::semantics::RegisterValuesViewConcept;
    { storage.store(state.get_state(), module_, memory, storage.registers(registers), arguments, caller) } -> std::same_as<StorageStoredState<Storage>>;
    { storage.store(state.get_state(), module_, memory, saved_registers, arguments, caller) } -> std::same_as<StorageStoredState<Storage>>;
    { storage.save_caller(state, memory) } -> std::same_as<CallStackView<Kind>>;
    { storage.materialize(state) } -> std::same_as<ProgramStateView<Kind>>;
    { storage.successor(state.get_state(), binding) } -> std::same_as<tyr::planning::Node<Kind, decltype(state.get_state())>>;
    { storage.successor(state.get_state(), borrowed_binding) } -> std::same_as<tyr::planning::Node<Kind, decltype(state.get_state())>>;
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

    InternedExecutionStorage(runir::kr::TaskContextPtr<Kind> context, ProgramView program) : m_context(std::move(context)), m_program(program) {}

    runir::kr::dl::semantics::RegisterValuesView registers(urd::data<runir::kr::dl::semantics::RegisterValues>& data)
    { return insert(*m_context->dl_denotation_repository, data).first; }

    [[nodiscard]] StateView view(StoredState state) const noexcept { return state; }
    [[nodiscard]] StoredState retain(StateView state) const noexcept { return state; }

    StoredState store(tyr::planning::StateView<Kind> planning_state,
                      ModuleView module_,
                      MemoryStateView memory_state,
                      runir::kr::dl::semantics::RegisterValuesView registers,
                      runir::kr::dl::semantics::CallArgumentsView arguments,
                      std::optional<CallStackView<Kind>> caller)
    {
        auto module_data = urd::data<ModuleState<Kind>>(planning_state.index(), module_.index(), memory_state.index(), registers.index(), arguments.index());
        auto data = urd::data<ProgramState<Kind>>(m_program.index(), insert(*m_context->execution_repository, module_data).first.index(), {});
        data.call_stack = urd::to_stored(caller);
        return insert(*m_context->execution_repository, data).first;
    }

    StoredState initial_state(const tyr::planning::StateView<Kind>& state)
    {
        const auto entry = m_program.get_entry_module();
        auto empty = checkout<runir::kr::dl::semantics::RegisterValues>(m_context->dl_builder);
        empty->concept_values.resize(entry.data().concept_registers.size());
        empty->role_values.resize(entry.data().role_registers.size());
        auto arguments = checkout<runir::kr::dl::semantics::CallArguments>(m_context->dl_builder);
        return store(state, entry, entry.get_entry_memory_state(), registers(*empty), insert(*m_context->dl_denotation_repository, *arguments).first, {});
    }

    /// Continue from an interned program state of this task and program.
    [[nodiscard]] StoredState start_state(ProgramStateView<Kind> state) const noexcept { return state; }

    ProgramStateView<Kind> materialize(ProgramStateView<Kind> state) { return state; }

    CallStackView<Kind> save_caller(ProgramStateView<Kind> state, MemoryStateView return_state)
    {
        const auto& module_ = state.get_module_state().data();
        auto saved = urd::data<CallStack>(module_.module_, return_state.index(), module_.registers, module_.arguments, state.data().call_stack);
        return insert(*m_context->execution_repository, saved).first;
    }

    template<ygg::RowViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>> Binding>
    tyr::planning::Node<Kind> successor(const tyr::planning::StateView<Kind>& state, Binding binding)
    {
        auto& search = *m_context->search_context;
        return search.successor_generator->get_successor_node(tyr::planning::Node<Kind>(state, 0), binding, *search.state_repository, *search.axiom_evaluator);
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
    urd::builder<tyr::planning::State<Kind>> m_planning;
    urd::Store<urd::slots<urd::builder<ProgramState<Kind>>>> m_program_states;
    runir::kr::TaskContextPtr<Kind> m_context;
    ProgramView m_program;

public:
    using StoredState = std::shared_ptr<urd::builder<ProgramState<Kind>>>;
    using StateView = BuilderProgramStateView<Kind>;

    TransientExecutionStorage(runir::kr::TaskContextPtr<Kind> context, ProgramView program) : m_context(std::move(context)), m_program(program) {}

    StateView view(const StoredState& state) const noexcept { return urd::resolve(*state, *m_context->execution_repository); }
    StoredState retain(StateView state)
    {
        auto result = m_program_states.make_shared();
        *result = state.data();
        return result;
    }
    runir::kr::dl::semantics::BorrowedRegisterValuesView registers(const urd::data<runir::kr::dl::semantics::RegisterValues>& data)
    { return urd::resolve(data, *m_context->search_context->task->get_repository()); }
    template<tyr::planning::StateViewConcept<Kind> S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
    StoredState store(const S& planning_state,
                      ModuleView module_,
                      MemoryStateView memory_state,
                      R registers,
                      runir::kr::dl::semantics::CallArgumentsView arguments,
                      std::optional<CallStackView<Kind>> caller)
    {
        auto result = m_program_states.make_shared();
        result->program = m_program.index();
        // Assign into the destination's existing buffers so released pool slots retain their capacity.
        auto& target = result->module_state;
        target.state.assign(planning_state.data());
        target.module_ = module_.index();
        target.memory_state = memory_state.index();
        runir::kr::dl::semantics::assign(target.registers, registers);
        target.arguments = arguments.index();
        result->call_stack = urd::to_stored(caller);
        return result;
    }

    StoredState initial_state(const tyr::planning::StateView<Kind>& state)
    {
        const auto entry = m_program.get_entry_module();
        auto empty = checkout<runir::kr::dl::semantics::RegisterValues>(m_context->dl_builder);
        empty->concept_values.resize(entry.data().concept_registers.size());
        empty->role_values.resize(entry.data().role_registers.size());
        auto arguments = checkout<runir::kr::dl::semantics::CallArguments>(m_context->dl_builder);
        return store(state, entry, entry.get_entry_memory_state(), registers(*empty), insert(*m_context->dl_denotation_repository, *arguments).first, {});
    }

    /// Continue from an interned program state of this task and program.
    StoredState start_state(ProgramStateView<Kind> state)
    {
        const auto module_ = state.get_module_state();
        return store(module_.get_state(),
                     module_.get_module(),
                     module_.get_memory_state(),
                     module_.get_registers(),
                     module_.get_arguments(),
                     state.get_call_stack());
    }

    ProgramStateView<Kind> materialize(BuilderProgramStateView<Kind> state)
    {
        auto& search = *m_context->search_context;
        const auto planning = search.state_repository->register_extended_state(state.get_state());
        auto& denotations = *m_context->dl_denotation_repository;
        auto& dl_builder = m_context->dl_builder;
        const auto module_ = state.get_module_state();
        auto data = urd::data<ModuleState<Kind>>(planning.index(),
                                                 module_.get_module().index(),
                                                 module_.get_memory_state().index(),
                                                 copy(module_.get_registers(), { denotations, dl_builder }).first.index(),
                                                 module_.get_arguments().index());
        auto program = urd::data<ProgramState<Kind>>(m_program.index(), insert(*m_context->execution_repository, data).first.index(), state.data().call_stack);
        return insert(*m_context->execution_repository, program).first;
    }

    CallStackView<Kind> save_caller(BuilderProgramStateView<Kind> state, MemoryStateView return_state)
    {
        const auto module_ = state.get_module_state();
        const auto registers = copy(module_.get_registers(), { *m_context->dl_denotation_repository, m_context->dl_builder }).first;
        auto saved = urd::data<CallStack>(module_.get_module().index(),
                                          return_state.index(),
                                          registers.index(),
                                          module_.get_arguments().index(),
                                          state.data().call_stack);
        return insert(*m_context->execution_repository, saved).first;
    }

    /// The returned state borrows scratch storage until the next successor() call.
    /// Consume or copy it before then; the source must not borrow this same scratch storage.
    template<ygg::RowViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>> Binding>
    tyr::planning::Node<Kind, tyr::planning::BuilderStateView<Kind>> successor(const tyr::planning::BuilderStateView<Kind>& state, Binding binding)
    {
        auto& search = *m_context->search_context;
        return search.successor_generator->get_successor_node(tyr::planning::Node<Kind, tyr::planning::BuilderStateView<Kind>>(state, 0),
                                                              binding,
                                                              m_planning,
                                                              *search.axiom_evaluator);
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
