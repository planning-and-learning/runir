#ifndef RUNIR_KR_PS_EXT_EXECUTION_STORAGE_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_STORAGE_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/detail/transient_state.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"

#include <concepts>
#include <optional>
#include <utility>
#include <vector>

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
    auto arguments() { return checkout<runir::kr::dl::semantics::CallArguments>(m_context->dl_builder); }
    auto registers(ygg::Data<runir::kr::dl::semantics::RegisterValues>& data) { return get_or_create(*m_context->dl_denotation_repository, data).first; }

    void set_registers(ygg::Data<ModuleState<Kind>>& target, runir::kr::dl::semantics::RegisterValuesView values) { target.registers = values.get_index(); }
    void set_arguments(ygg::Data<ModuleState<Kind>>& target, ygg::UniqueObjectPoolPtr<ygg::Data<runir::kr::dl::semantics::CallArguments>> arguments)
    {
        target.arguments = get_or_create(*m_context->dl_denotation_repository, *arguments).first.get_index();
    }

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
        set_arguments(*data, arguments());
        return store(std::move(data), {});
    }

    ProgramStateView<Kind> materialize(ProgramStateView<Kind> state) { return state; }

    template<typename FeatureTag, typename C, runir::kr::dl::semantics::StateEvaluationContextConcept Context>
    void append_call_argument(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> argument,
                              Context& context,
                              ygg::Data<runir::kr::dl::semantics::CallArguments>& target)
    {
        const auto denotation = evaluate(argument.get_expression(), context, context.get_denotation_repository());
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
            target.concept_arguments.push_back(denotation.get_index());
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
            target.role_arguments.push_back(denotation.get_index());
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
            target.boolean_arguments.push_back(denotation.get_index());
        else
            target.numerical_arguments.push_back(denotation.get_index());
    }

    static bool arguments_match(ModuleView callee, const ygg::Data<runir::kr::dl::semantics::CallArguments>& arguments)
    {
        return arguments.concept_arguments.size() == callee.template get_arguments<runir::kr::dl::ConceptTag>().size()
               && arguments.role_arguments.size() == callee.template get_arguments<runir::kr::dl::RoleTag>().size()
               && arguments.boolean_arguments.size() == callee.template get_arguments<runir::kr::dl::BooleanTag>().size()
               && arguments.numerical_arguments.size() == callee.template get_arguments<runir::kr::dl::NumericalTag>().size();
    }

    auto save_caller(ProgramStateView<Kind> state, MemoryStateView return_state)
    {
        const auto& module_ = state.get_module_state().get_data();
        auto saved = ygg::Data<CallStack>(module_.module_, return_state.get_index(), module_.registers, module_.arguments, state.get_data().call_stack);
        return get_or_create(*m_context->execution_repository, saved).first;
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto choice(RuleVariantView rule, runir::kr::dl::semantics::DenotationView<Category> denotation)
    {
        return detail::Choice<Category>(rule, denotation);
    }
    auto& choice_denotation_repository(runir::kr::dl::semantics::DenotationRepository&) { return *m_context->dl_denotation_repository; }

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

/// Pooled construction used by NONE and CHOICE; only selected output paths are materialized.
template<tyr::TaskKind Kind>
class TransientExecutionStorage
{
    detail::TransientPools<Kind> m_pools;
    runir::kr::TaskContextPtr<Kind> m_context;
    ProgramView m_program;

public:
    TransientExecutionStorage(runir::kr::TaskContextPtr<Kind> context, ProgramView program) : m_context(std::move(context)), m_program(program) {}

    auto module_() { return m_pools.module_(); }
    auto arguments() { return m_pools.arguments(); }
    auto registers(const ygg::Data<runir::kr::dl::semantics::RegisterValues>& data)
    {
        return detail::RegisterValuesRef(data, *m_context->search_context->task->get_repository());
    }
    void set_registers(detail::TransientModuleData<Kind>& target, detail::RegisterValuesRef values) { target.registers = values.get_data(); }
    void set_arguments(detail::TransientModuleData<Kind>& target, detail::TransientArgumentsPtr arguments) { target.arguments = std::move(arguments); }

    detail::TransientProgramState<Kind> store(detail::TransientModulePtr<Kind> module_, detail::TransientCallerPtr caller)
    {
        return detail::TransientProgramState<Kind>(m_context, m_program, std::move(module_), std::move(caller));
    }

    detail::TransientProgramState<Kind> initial_state(const tyr::planning::StateView<Kind>& state)
    {
        const auto entry = m_program.get_entry_module();
        auto data = module_();
        data->state = m_pools.planning();
        *data->state->builder = state.get_state_builder();
        data->module_ = entry.get_index();
        data->memory_state = entry.get_entry_memory_state().get_index();
        auto empty = checkout<runir::kr::dl::semantics::RegisterValues>(m_context->dl_builder);
        empty->concept_values.resize(entry.template get_registers<runir::kr::dl::ConceptTag>().size());
        empty->role_values.resize(entry.template get_registers<runir::kr::dl::RoleTag>().size());
        set_registers(*data, registers(*empty));
        set_arguments(*data, arguments());
        return store(std::move(data), {});
    }

    ProgramStateView<Kind> materialize(const detail::TransientProgramState<Kind>& state)
    {
        auto& search = *m_context->search_context;
        auto builder = search.state_repository->get_state_builder();
        *builder = state.get_state().get_state_builder();
        const auto planning = search.state_repository->register_extended_state(std::move(builder));
        auto& denotations = *m_context->dl_denotation_repository;
        auto& dl_builder = m_context->dl_builder;
        auto caller = std::optional<CallStackView<Kind>> {};
        auto callers = std::vector<detail::TransientCallerPtr> {};
        for (auto current = state.get_call_stack(); current; current = current->caller)
            callers.push_back(current);
        for (auto it = callers.rbegin(); it != callers.rend(); ++it)
        {
            const auto& saved = **it;
            auto data = ygg::Data<CallStack>(saved.module_,
                                             saved.return_memory_state,
                                             detail::materialize_register_values(saved.registers, dl_builder, denotations).get_index(),
                                             detail::materialize_call_arguments(*saved.arguments, dl_builder, denotations).get_index());
            ygg::set(caller, data.caller);
            caller = get_or_create(*m_context->execution_repository, data).first;
        }
        const auto& module_ = state.get_module_state().get_data();
        auto data = ygg::Data<ModuleState<Kind>>(planning.get_index(),
                                                 module_.module_,
                                                 module_.memory_state,
                                                 detail::materialize_register_values(module_.registers, dl_builder, denotations).get_index(),
                                                 detail::materialize_call_arguments(*module_.arguments, dl_builder, denotations).get_index());
        auto program = ygg::Data<ProgramState<Kind>>(m_program.get_index(), get_or_create(*m_context->execution_repository, data).first.get_index());
        ygg::set(caller, program.call_stack);
        return get_or_create(*m_context->execution_repository, program).first;
    }

    template<typename FeatureTag, typename C, runir::kr::dl::semantics::StateEvaluationContextConcept Context>
    void append_call_argument(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> argument,
                              Context& context,
                              detail::OwnedCallArguments& target)
    {
        const auto denotation = evaluate(argument.get_expression(), context);
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag> || std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
            detail::append_call_argument<FeatureTag>(target, denotation);
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
            detail::append_call_argument<runir::kr::dl::BooleanTag>(target, denotation);
        else
            detail::append_call_argument<runir::kr::dl::NumericalTag>(target, denotation);
    }

    static bool arguments_match(ModuleView callee, const detail::OwnedCallArguments& arguments)
    {
        return arguments.template get<runir::kr::dl::ConceptTag>().size() == callee.template get_arguments<runir::kr::dl::ConceptTag>().size()
               && arguments.template get<runir::kr::dl::RoleTag>().size() == callee.template get_arguments<runir::kr::dl::RoleTag>().size()
               && arguments.template get<runir::kr::dl::BooleanTag>().size() == callee.template get_arguments<runir::kr::dl::BooleanTag>().size()
               && arguments.template get<runir::kr::dl::NumericalTag>().size() == callee.template get_arguments<runir::kr::dl::NumericalTag>().size();
    }

    auto save_caller(const detail::TransientProgramState<Kind>& state, MemoryStateView return_state)
    {
        const auto& module_ = state.get_module_state().get_data();
        auto saved = m_pools.caller();
        saved->module_ = module_.module_;
        saved->return_memory_state = return_state.get_index();
        saved->registers = module_.registers;
        saved->arguments = module_.arguments;
        saved->caller = state.get_call_stack();
        return saved;
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto choice(RuleVariantView rule, runir::kr::dl::semantics::DenotationView<Category> denotation)
    {
        return m_pools.template choice<Category>(rule, denotation);
    }
    auto& choice_denotation_repository(runir::kr::dl::semantics::DenotationRepository& scratch) { return scratch; }

    template<tyr::planning::StateViewConcept<Kind> S>
    detail::TransientLabeledNode<Kind> successor(const S& state, tyr::formalism::planning::ActionBindingView binding)
    {
        auto& search = *m_context->search_context;
        auto target = m_pools.planning();
        target->builder->clear();
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
            target->builder->resize_derived_atoms(search.task->get_task().template get_atoms<tyr::formalism::DerivedTag>().size());
        search.successor_generator->generate_successor_state(tyr::planning::Node<S>(state, 0), binding, *target->builder);
        search.axiom_evaluator->compute_extended_state(*target->builder);
        const auto view = tyr::planning::BuilderStateView<Kind>(*target->builder, *search.task);
        return { std::move(target), binding, tyr::planning::Node<tyr::planning::BuilderStateView<Kind>>(view, 0) };
    }
    void set_planning_state(detail::TransientModuleData<Kind>& target, const detail::TransientLabeledNode<Kind>& successor) { target.state = successor.owner; }
};

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class InternedExecutionStorage<tyr::GroundTag>;

extern template tyr::planning::LabeledNode<tyr::planning::StateView<tyr::GroundTag>>
InternedExecutionStorage<tyr::GroundTag>::successor<tyr::planning::StateView<tyr::GroundTag>>(const tyr::planning::StateView<tyr::GroundTag>&,
                                                                                              tyr::formalism::planning::ActionBindingView);

extern template class TransientExecutionStorage<tyr::GroundTag>;

extern template detail::TransientLabeledNode<tyr::GroundTag>
TransientExecutionStorage<tyr::GroundTag>::successor<tyr::planning::BuilderStateView<tyr::GroundTag>>(const tyr::planning::BuilderStateView<tyr::GroundTag>&,
                                                                                                      tyr::formalism::planning::ActionBindingView);

extern template class InternedExecutionStorage<tyr::LiftedTag>;

extern template tyr::planning::LabeledNode<tyr::planning::StateView<tyr::LiftedTag>>
InternedExecutionStorage<tyr::LiftedTag>::successor<tyr::planning::StateView<tyr::LiftedTag>>(const tyr::planning::StateView<tyr::LiftedTag>&,
                                                                                              tyr::formalism::planning::ActionBindingView);

extern template class TransientExecutionStorage<tyr::LiftedTag>;

extern template detail::TransientLabeledNode<tyr::LiftedTag>
TransientExecutionStorage<tyr::LiftedTag>::successor<tyr::planning::BuilderStateView<tyr::LiftedTag>>(const tyr::planning::BuilderStateView<tyr::LiftedTag>&,
                                                                                                      tyr::formalism::planning::ActionBindingView);

#endif

}  // namespace runir::kr::ps::ext

#endif
