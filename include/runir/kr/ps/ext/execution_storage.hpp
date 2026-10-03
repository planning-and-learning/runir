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
#include <yggdrasil/containers/unique_object_pool.hpp>

namespace runir::kr::ps::ext
{

template<typename Storage, tyr::planning::StateViewConcept S, runir::kr::dl::semantics::RegisterValuesViewConcept R>
auto make_module(Storage& storage,
                 const S& planning_state,
                 ModuleView module_,
                 MemoryStateView memory_state,
                 R registers,
                 runir::kr::dl::semantics::CallArgumentsView arguments)
{
    auto result = storage.module_();
    storage.set_planning_state(*result, planning_state);
    result->module_ = module_.get_index();
    result->memory_state = memory_state.get_index();
    storage.set_registers(*result, registers);
    result->arguments = arguments.get_index();
    return result;
}

/// Repository-backed construction used when the full explored graph is retained.
template<tyr::TaskKind Kind>
class InternedExecutionStorage
{
    runir::kr::TaskContextPtr<Kind> m_context;
    ProgramView m_program;

public:
    InternedExecutionStorage(runir::kr::TaskContextPtr<Kind> context, ProgramView program) : m_context(std::move(context)), m_program(program) {}

    auto module_() { return checkout<ModuleState<Kind>>(m_context->execution_builder); }
    auto registers(ygg::Data<runir::kr::dl::semantics::RegisterValues>& data) { return get_or_create(*m_context->dl_denotation_repository, data).first; }

    auto view(ProgramStateView<Kind> state) const noexcept { return state; }
    auto retain(ProgramStateView<Kind> state) const noexcept { return state; }

    void set_registers(ygg::Data<ModuleState<Kind>>& target, runir::kr::dl::semantics::RegisterValuesView values) { target.registers = values.get_index(); }

    ProgramStateView<Kind> store(ygg::UniqueObjectPoolPtr<ygg::Data<ModuleState<Kind>>> module_state, std::optional<CallStackView<Kind>> caller)
    {
        auto data = ygg::Data<ProgramState<Kind>>(m_program.get_index(), get_or_create(*m_context->execution_repository, *module_state).first.get_index());
        ygg::set(caller, data.call_stack);
        return get_or_create(*m_context->execution_repository, data).first;
    }

    ProgramStateView<Kind> initial_state(const tyr::planning::StateView<Kind>& state)
    {
        const auto entry = m_program.get_entry_module();
        auto data = module_();
        data->state = state.get_index();
        data->module_ = entry.get_index();
        data->memory_state = entry.get_entry_memory_state().get_index();
        auto empty = checkout<runir::kr::dl::semantics::RegisterValues>(m_context->dl_builder);
        empty->concept_values.resize(entry.template get_registers<runir::kr::dl::ConceptTag>().size());
        empty->role_values.resize(entry.template get_registers<runir::kr::dl::RoleTag>().size());
        set_registers(*data, registers(*empty));
        auto arguments = checkout<runir::kr::dl::semantics::CallArguments>(m_context->dl_builder);
        data->arguments = get_or_create(*m_context->dl_denotation_repository, *arguments).first.get_index();
        return store(std::move(data), {});
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
    void set_planning_state(ygg::Data<ModuleState<Kind>>& target, tyr::planning::StateView<Kind> state) { target.state = state.get_index(); }
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

template<tyr::TaskKind Kind, typename Storage, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
auto planning_step(Storage& storage,
                   S state,
                   const tyr::planning::LabeledNode<PS>& successor,
                   RuleVariantView rule,
                   MemoryStateView memory_state,
                   const runir::kr::TaskContextPtr<Kind>& task_context)
{
    const auto module_ = state.get_module_state();
    auto target = make_module(storage, successor.node.get_state(), module_.get_module(), memory_state, module_.get_registers(), module_.get_arguments());
    auto step = applied(storage.store(std::move(target), state.get_call_stack()), rule, task_context);
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
    ygg::UniqueObjectPool<ygg::Builder<ModuleState<Kind>>> m_module_states;
    ygg::SharedObjectPool<ygg::Builder<ProgramState<Kind>>> m_program_states;
    runir::kr::TaskContextPtr<Kind> m_context;
    ProgramView m_program;

public:
    TransientExecutionStorage(runir::kr::TaskContextPtr<Kind> context, ProgramView program) : m_context(std::move(context)), m_program(program) {}

    auto module_() { return m_module_states.get_or_allocate(); }

    auto view(const ygg::SharedObjectPoolPtr<ygg::Builder<ProgramState<Kind>>>& state) const noexcept
    {
        return ygg::make_view(*state, *m_context->execution_repository);
    }
    auto retain(BuilderProgramStateView<Kind> state)
    {
        auto result = m_program_states.get_or_allocate();
        *result = state.get_data();
        return result;
    }
    auto registers(const ygg::Data<runir::kr::dl::semantics::RegisterValues>& data)
    {
        return ygg::make_view(data, *m_context->search_context->task->get_repository());
    }
    template<runir::kr::dl::semantics::RegisterValuesViewConcept R>
    void set_registers(ygg::Builder<ModuleState<Kind>>& target, R values)
    {
        target.registers = values.get_data();
    }

    auto store(ygg::UniqueObjectPoolPtr<ygg::Builder<ModuleState<Kind>>> module_state, std::optional<CallStackView<Kind>> call_stack)
    {
        auto result = m_program_states.get_or_allocate();
        result->program = m_program.get_index();
        // Exchange buffers so both reusable pool slots retain their allocations.
        std::swap(result->module_state, *module_state);
        ygg::set(call_stack, result->call_stack);
        return result;
    }

    auto initial_state(const tyr::planning::StateView<Kind>& state)
    {
        const auto entry = m_program.get_entry_module();
        auto data = module_();
        data->state = state.get_state_builder();
        data->module_ = entry.get_index();
        data->memory_state = entry.get_entry_memory_state().get_index();
        auto empty = checkout<runir::kr::dl::semantics::RegisterValues>(m_context->dl_builder);
        empty->concept_values.resize(entry.template get_registers<runir::kr::dl::ConceptTag>().size());
        empty->role_values.resize(entry.template get_registers<runir::kr::dl::RoleTag>().size());
        set_registers(*data, registers(*empty));
        auto arguments = checkout<runir::kr::dl::semantics::CallArguments>(m_context->dl_builder);
        data->arguments = get_or_create(*m_context->dl_denotation_repository, *arguments).first.get_index();
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
    void set_planning_state(ygg::Builder<ModuleState<Kind>>& target, tyr::planning::BuilderStateView<Kind> state) { target.state = state.get_state_builder(); }
};

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class InternedExecutionStorage<tyr::GroundTag>;
extern template class InternedExecutionStorage<tyr::LiftedTag>;
extern template class TransientExecutionStorage<tyr::GroundTag>;
extern template class TransientExecutionStorage<tyr::LiftedTag>;

#endif

}  // namespace runir::kr::ps::ext

#endif
