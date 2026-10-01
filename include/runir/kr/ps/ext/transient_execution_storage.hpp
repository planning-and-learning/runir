#ifndef RUNIR_KR_PS_EXT_TRANSIENT_EXECUTION_STORAGE_HPP_
#define RUNIR_KR_PS_EXT_TRANSIENT_EXECUTION_STORAGE_HPP_

#include "runir/kr/dl/semantics/evaluation.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"
#include "runir/kr/task_context.hpp"

#include <concepts>
#include <optional>
#include <utility>
#include <vector>
#include <yggdrasil/containers/shared_object_pool.hpp>
#include <yggdrasil/containers/unique_object_pool.hpp>

namespace runir::kr::ps::ext
{

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

template<tyr::TaskKind Kind>
struct TransientLabeledNode
{
    ygg::SharedObjectPoolPtr<ygg::Builder<tyr::planning::State<Kind>>> owner;
    tyr::formalism::planning::ActionBindingView label;
    tyr::planning::Node<tyr::planning::BuilderStateView<Kind>> node;
};

/// Owns the pools; all checked-out handles must be released before this object is destroyed.
template<tyr::TaskKind Kind>
class TransientPools
{
    ygg::SharedObjectPool<ygg::Builder<tyr::planning::State<Kind>>> m_planning;
    ygg::SharedObjectPool<ygg::Builder<ModuleState<Kind>>> m_module_states;
    ygg::SharedObjectPool<ygg::Builder<CallStack>> m_call_stacks;

public:
    auto planning() { return m_planning.get_or_allocate(); }
    auto module_() { return m_module_states.get_or_allocate(); }
    auto caller() { return m_call_stacks.get_or_allocate(); }
};

}  // namespace detail

/// Pooled per-state construction used by NONE and CHOICE; only selected output paths are materialized.
/// Call arguments and their final denotations are always interned in the task repository.
/// Returned transient states and handles must be released before this storage is destroyed.
template<tyr::TaskKind Kind>
class TransientExecutionStorage
{
    detail::TransientPools<Kind> m_pools;
    runir::kr::TaskContextPtr<Kind> m_context;
    ProgramView m_program;

public:
    TransientExecutionStorage(runir::kr::TaskContextPtr<Kind> context, ProgramView program) : m_context(std::move(context)), m_program(program) {}

    auto module_() { return m_pools.module_(); }

    auto view(const ygg::Builder<ProgramState<Kind>>& state) const noexcept { return ygg::make_view(state, *m_context->execution_repository); }
    auto retain(BuilderProgramStateView<Kind> state) const { return state.get_data(); }
    auto call_stack(BuilderProgramStateView<Kind> state) const { return state.get_data().call_stack; }
    auto caller(BuilderCallStackView<Kind> state) const { return state.get_data().caller; }
    auto registers(const ygg::Data<runir::kr::dl::semantics::RegisterValues>& data)
    {
        return ygg::make_view(data, *m_context->search_context->task->get_repository());
    }
    void set_registers(ygg::Builder<ModuleState<Kind>>& target, runir::kr::dl::semantics::BorrowedRegisterValuesView values)
    {
        target.registers = values.get_data();
    }
    void set_arguments(ygg::Builder<ModuleState<Kind>>& target, runir::kr::dl::semantics::CallArgumentsView arguments) { target.arguments = arguments; }
    void set_module(ygg::Builder<ModuleState<Kind>>& target, ModuleView module_) { target.module_ = module_; }
    void set_memory_state(ygg::Builder<ModuleState<Kind>>& target, MemoryStateView memory_state) { target.memory_state = memory_state; }

    ygg::Builder<ProgramState<Kind>> store(ygg::SharedObjectPoolPtr<ygg::Builder<ModuleState<Kind>>> module_state,
                                           ygg::SharedObjectPoolPtr<ygg::Builder<CallStack>> call_stack)
    {
        return ygg::Builder<ProgramState<Kind>>(m_program, std::move(module_state), std::move(call_stack));
    }

    ygg::Builder<ProgramState<Kind>> initial_state(const tyr::planning::StateView<Kind>& state)
    {
        const auto entry = m_program.get_entry_module();
        auto data = module_();
        data->state = m_pools.planning();
        *data->state = state.get_state_builder();
        data->module_ = entry;
        data->memory_state = entry.get_entry_memory_state();
        auto empty = checkout<runir::kr::dl::semantics::RegisterValues>(m_context->dl_builder);
        empty->concept_values.resize(entry.template get_registers<runir::kr::dl::ConceptTag>().size());
        empty->role_values.resize(entry.template get_registers<runir::kr::dl::RoleTag>().size());
        set_registers(*data, registers(*empty));
        auto arguments = checkout<runir::kr::dl::semantics::CallArguments>(m_context->dl_builder);
        data->arguments = get_or_create(*m_context->dl_denotation_repository, *arguments).first;
        return store(std::move(data), {});
    }

    ProgramStateView<Kind> materialize(BuilderProgramStateView<Kind> state)
    {
        auto& search = *m_context->search_context;
        auto builder = search.state_repository->get_state_builder();
        *builder = state.get_state().get_state_builder();
        const auto planning = search.state_repository->register_extended_state(std::move(builder));
        auto& denotations = *m_context->dl_denotation_repository;
        auto& dl_builder = m_context->dl_builder;
        auto caller = std::optional<CallStackView<Kind>> {};
        auto callers = std::vector<BuilderCallStackView<Kind>> {};
        for (auto current = state.get_call_stack(); current; current = current->get_caller())
            callers.push_back(*current);
        for (auto it = callers.rbegin(); it != callers.rend(); ++it)
        {
            const auto saved = *it;
            auto data = ygg::Data<CallStack>(saved.get_module().get_index(),
                                             saved.get_return_memory_state().get_index(),
                                             detail::materialize_register_values(saved.get_registers(), dl_builder, denotations).get_index(),
                                             saved.get_arguments().get_index());
            ygg::set(caller, data.caller);
            caller = get_or_create(*m_context->execution_repository, data).first;
        }
        const auto module_ = state.get_module_state();
        auto data = ygg::Data<ModuleState<Kind>>(planning.get_index(),
                                                 module_.get_module().get_index(),
                                                 module_.get_memory_state().get_index(),
                                                 detail::materialize_register_values(module_.get_registers(), dl_builder, denotations).get_index(),
                                                 module_.get_arguments().get_index());
        auto program = ygg::Data<ProgramState<Kind>>(m_program.get_index(), get_or_create(*m_context->execution_repository, data).first.get_index());
        ygg::set(caller, program.call_stack);
        return get_or_create(*m_context->execution_repository, program).first;
    }

    auto save_caller(BuilderProgramStateView<Kind> state, MemoryStateView return_state)
    {
        const auto module_ = state.get_module_state();
        auto saved = m_pools.caller();
        saved->module_ = module_.get_module();
        saved->return_memory_state = return_state;
        saved->registers = module_.get_data().registers;
        saved->arguments = module_.get_data().arguments;
        saved->caller = call_stack(state);
        return saved;
    }

    template<tyr::planning::StateViewConcept<Kind> S>
    detail::TransientLabeledNode<Kind> successor(const S& state, tyr::formalism::planning::ActionBindingView binding)
    {
        auto& search = *m_context->search_context;
        auto target = m_pools.planning();
        target->clear();
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            target->resize_derived_atoms(search.task->get_task().template get_atoms<tyr::formalism::DerivedTag>().size());
        search.successor_generator->generate_successor_state(tyr::planning::Node<S>(state, 0), binding, *target);
        search.axiom_evaluator->compute_extended_state(*target);
        const auto view = tyr::planning::BuilderStateView<Kind>(*target, *search.task);
        return { std::move(target), binding, tyr::planning::Node<tyr::planning::BuilderStateView<Kind>>(view, 0) };
    }
    void set_planning_state(ygg::Builder<ModuleState<Kind>>& target, const detail::TransientLabeledNode<Kind>& successor) { target.state = successor.owner; }
};

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class TransientExecutionStorage<tyr::GroundTag>;

extern template detail::TransientLabeledNode<tyr::GroundTag>
TransientExecutionStorage<tyr::GroundTag>::successor<tyr::planning::BuilderStateView<tyr::GroundTag>>(const tyr::planning::BuilderStateView<tyr::GroundTag>&,
                                                                                                      tyr::formalism::planning::ActionBindingView);

extern template class TransientExecutionStorage<tyr::LiftedTag>;

extern template detail::TransientLabeledNode<tyr::LiftedTag>
TransientExecutionStorage<tyr::LiftedTag>::successor<tyr::planning::BuilderStateView<tyr::LiftedTag>>(const tyr::planning::BuilderStateView<tyr::LiftedTag>&,
                                                                                                      tyr::formalism::planning::ActionBindingView);

#endif

}  // namespace runir::kr::ps::ext

#endif
