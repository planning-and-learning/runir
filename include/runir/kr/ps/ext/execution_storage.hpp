#ifndef RUNIR_KR_PS_EXT_EXECUTION_STORAGE_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_STORAGE_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/task_context.hpp"

#include <concepts>
#include <optional>
#include <utility>

namespace runir::kr::ps::ext
{

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
    auto call_stack(ProgramStateView<Kind> state) const { return state.get_call_stack(); }
    auto caller(CallStackView<Kind> state) const { return state.get_caller(); }

    void set_registers(ygg::Data<ModuleState<Kind>>& target, runir::kr::dl::semantics::RegisterValuesView values) { target.registers = values.get_index(); }
    void set_arguments(ygg::Data<ModuleState<Kind>>& target, runir::kr::dl::semantics::CallArgumentsView arguments)
    {
        target.arguments = arguments.get_index();
    }
    void set_module(ygg::Data<ModuleState<Kind>>& target, ModuleView module_) { target.module_ = module_.get_index(); }
    void set_memory_state(ygg::Data<ModuleState<Kind>>& target, MemoryStateView memory_state) { target.memory_state = memory_state.get_index(); }

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

    template<tyr::planning::StateViewConcept<Kind> S>
    tyr::planning::LabeledNode<tyr::planning::StateView<Kind>> successor(const S& state, tyr::formalism::planning::ActionBindingView binding)
    {
        auto& search = *m_context->search_context;
        return { binding,
                 search.successor_generator->get_successor_node(tyr::planning::Node<S>(state, 0), binding, *search.state_repository, *search.axiom_evaluator) };
    }
    void set_planning_state(ygg::Data<ModuleState<Kind>>& target, const tyr::planning::LabeledNode<tyr::planning::StateView<Kind>>& successor)
    {
        target.state = successor.node.get_state().get_index();
    }
};

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class InternedExecutionStorage<tyr::GroundTag>;

extern template tyr::planning::LabeledNode<tyr::planning::StateView<tyr::GroundTag>>
InternedExecutionStorage<tyr::GroundTag>::successor<tyr::planning::StateView<tyr::GroundTag>>(const tyr::planning::StateView<tyr::GroundTag>&,
                                                                                              tyr::formalism::planning::ActionBindingView);

extern template class InternedExecutionStorage<tyr::LiftedTag>;

extern template tyr::planning::LabeledNode<tyr::planning::StateView<tyr::LiftedTag>>
InternedExecutionStorage<tyr::LiftedTag>::successor<tyr::planning::StateView<tyr::LiftedTag>>(const tyr::planning::StateView<tyr::LiftedTag>&,
                                                                                              tyr::formalism::planning::ActionBindingView);

#endif

}  // namespace runir::kr::ps::ext

#endif
