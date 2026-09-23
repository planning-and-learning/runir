#ifndef RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/action_rule.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <stdexcept>
#include <tyr/formalism/planning/action_view.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/containers/variant.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
class SuccessorExpander
{
public:
    using LabeledNode = tyr::planning::LabeledNode<Kind>;
    using Step = detail::ProgramStep<Kind>;
    using Expansion = std::variant<Step, detail::Choice<runir::kr::dl::ConceptTag>, detail::Choice<runir::kr::dl::RoleTag>>;

    SuccessorExpander(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program) :
        m_task_context(task_context ? std::move(task_context) : throw std::invalid_argument("SuccessorExpander requires a task context.")),
        m_program(program),
        m_environment(*m_task_context, m_program),
        m_action_rule_evaluator(m_task_context->search_context->task)
    {
        if (&m_program.get_context() != m_task_context->domain_context->ext_repository.get())
            throw std::invalid_argument("SuccessorExpander requires a program from the domain context repository.");
    }

    const auto& get_task_context() const noexcept { return m_task_context; }

    /// Intern the entry module with empty registers and arguments, and no caller.
    ProgramStateView<Kind> initial_state(const tyr::planning::Node<Kind>& node)
    {
        validate_node(node);
        auto arguments = checkout<runir::kr::dl::semantics::CallArguments>(m_task_context->dl_builder);
        const auto module_ = m_program.get_entry_module();
        auto data = ygg::Data<ModuleState<Kind>>(node.get_state().get_index(),
                                                 module_.get_index(),
                                                 module_.get_entry_memory_state().get_index(),
                                                 empty_registers(module_).get_index(),
                                                 get_or_create(*m_task_context->dl_denotation_repository, *arguments).first.get_index());
        return intern(data, std::nullopt);
    }

    /// Emit a program step or a compact Choose obligation in natural rule and binding order.
    /// Return true on exhaustion; emit returning false or stop returning true ends enumeration.
    /// Count applied successors and caller returns, not Choice descriptors or failure markers.
    /// Callbacks must not reenter this expander or its successor generator. Apply choices after enumeration.
    template<typename Emit, typename Stop>
    bool for_each_successor(ProgramStateView<Kind> state, const tyr::planning::Node<Kind>& node, ProgramSearchStatistics& statistics, Emit&& emit, Stop&& stop)
    {
        if (stop())
            return false;
        validate_source(state, node);
        m_environment.get_dl_caches().clear(false);
        bool emitted = false;
        const auto emit_expansion = [&](Expansion expansion)
        {
            emitted = true;
            if (const auto* step = std::get_if<Step>(&expansion))
                statistics.num_generated += step->status == detail::ProgramOutcome::APPLIED || step->status == detail::ProgramOutcome::RESTORED_CALLER;
            return emit(std::move(expansion));
        };
        for (const auto transition : state.get_module_state().get_module().get_memory_transitions())
            for (const auto rule : transition)
            {
                if (stop())
                    return false;
                if (!ygg::visit([&](auto concrete) { return emit_rule(concrete, rule, state, node, emit_expansion, stop); }, rule.get_variant()))
                    return false;
            }
        if (stop())
            return false;
        return emitted || emit_expansion(fallback(state));
    }

    /// Apply the current admitted binding without advancing its cursor; an exhausted choice reports FAILURE.
    template<runir::kr::dl::CategoryTag Category>
    Step apply_choice(ProgramStateView<Kind> state,
                      const tyr::planning::Node<Kind>& node,
                      const detail::Choice<Category>& choice,
                      ProgramSearchStatistics& statistics)
    {
        validate_source(state, node);
        auto step = choice_step(state, choice);
        statistics.num_generated += step.status == detail::ProgramOutcome::APPLIED;
        return step;
    }

    /// Find the first rule that admits this planning successor; control-only rules do not match planning actions.
    std::optional<RuleVariantView> matching_rule(ProgramStateView<Kind> state, const tyr::planning::Node<Kind>& node, const LabeledNode& candidate)
    {
        validate_source(state, node);
        validate_node(candidate.node);
        m_environment.get_dl_caches().clear(false);
        m_environment.get_dl_target_caches().clear(false);
        for (const auto& transition : state.get_module_state().get_module().get_memory_transitions())
            for (auto rule : transition)
                if (ygg::visit([&](auto concrete) { return matches(concrete, state, node, candidate); }, rule.get_variant()))
                    return rule;
        return std::nullopt;
    }

    /// Apply one rule, using a supplied planning successor for Do, Action, or a Sketch with effects.
    /// Load, Choose, Call, and empty-effect Sketch rules derive their own control transition.
    std::optional<Step>
    apply(ProgramStateView<Kind> state, const tyr::planning::Node<Kind>& node, RuleVariantView rule, std::optional<LabeledNode> candidate = std::nullopt)
    {
        validate_source(state, node);
        if (candidate)
            validate_node(candidate->node);
        m_environment.get_dl_caches().clear(false);
        m_environment.get_dl_target_caches().clear(false);
        return ygg::visit([&](auto concrete) { return apply_rule(concrete, rule, state, node, candidate); }, rule.get_variant());
    }

private:
    // Validate borrowed views before evaluating features or modifying the execution repository.
    void validate_node(const tyr::planning::Node<Kind>& node) const
    {
        if (node.get_state().get_state_repository().get() != m_task_context->search_context->state_repository.get())
            throw std::invalid_argument("SuccessorExpander requires a planning node from the selected task's state repository.");
    }

    void validate_source(ProgramStateView<Kind> state, const tyr::planning::Node<Kind>& node) const
    {
        validate_node(node);
        if (&state.get_context() != m_task_context->execution_repository.get())
            throw std::invalid_argument("SuccessorExpander requires an execution state from the selected task.");
        const auto program = state.get_program();
        if (&program.get_context() != &m_program.get_context() || program.get_index() != m_program.get_index())
            throw std::invalid_argument("SuccessorExpander requires an execution state from the selected program.");
        const auto module_state = state.get_module_state();
        if (module_state.get_data().state != node.get_state().get_index())
            throw std::invalid_argument("Program state and planning node must identify the same planning state.");
    }

    runir::kr::dl::semantics::RegisterValuesView empty_registers(ModuleView module_)
    {
        auto data = checkout<runir::kr::dl::semantics::RegisterValues>(m_task_context->dl_builder);
        data->concept_values.resize(module_.template get_registers<runir::kr::dl::ConceptTag>().size());
        data->role_values.resize(module_.template get_registers<runir::kr::dl::RoleTag>().size());
        return get_or_create(*m_task_context->dl_denotation_repository, *data).first;
    }

    ProgramStateView<Kind> intern(ygg::Data<ModuleState<Kind>>& module_state, std::optional<CallStackView<Kind>> caller)
    {
        auto data = ygg::Data<ProgramState<Kind>>(m_program.get_index(), get_or_create(*m_task_context->execution_repository, module_state).first.get_index());
        ygg::set(caller, data.call_stack);
        return get_or_create(*m_task_context->execution_repository, data).first;
    }

    // Rule admission and argument/effect evaluation share the environment's reusable denotation caches.
    template<RuleKind RuleKindT, typename C>
    static bool has_current_source(ygg::View<ygg::Index<Rule<RuleKindT>>, C> rule, ProgramStateView<Kind> state)
    {
        return rule.get_source().get_index() == state.get_module_state().get_memory_state().get_index();
    }

    template<RuleKind RuleKindT, typename C>
    bool rule_is_applicable(ygg::View<ygg::Index<Rule<RuleKindT>>, C> rule, ProgramStateView<Kind> state, const tyr::planning::Node<Kind>& node)
    {
        if (!has_current_source(rule, state))
            return false;
        auto state_context =
            m_environment.make_dl_context(node.get_state(), state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        return runir::kr::ps::ext::conditions_are_compatible(rule, state_context);
    }

    template<BindingRuleKind RuleKindT, typename C, typename Value>
    static void
    apply_binding(ygg::View<ygg::Index<Rule<RuleKindT>>, C> rule, const Value& value, ygg::Data<runir::kr::dl::semantics::RegisterValues>& registers)
    {
        const auto index = size_t(ygg::uint_t(rule.get_register().get_identifier()));
        using Category = typename RuleKindT::Category;
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            registers.concept_values.at(index) = value.get_index();
        else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
            registers.role_values.at(index) = ::cista::pair(value.first.get_index(), value.second.get_index());
    }

    template<typename C>
    auto& evaluate_do_arguments(ygg::View<ygg::Index<Rule<DoTag>>, C> rule, ProgramStateView<Kind> state, const tyr::planning::Node<Kind>& node)
    {
        const auto arguments = rule.get_action_arguments();
        auto& denotations = m_environment.prepare_do_argument_denotations();
        auto state_context =
            m_environment.make_dl_context(node.get_state(), state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        for (auto argument : arguments)
            denotations.push_back(evaluate(argument, state_context));
        return denotations;
    }

    template<typename ConceptDenotations, typename C>
    static bool action_matches_do_arguments(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                                            tyr::formalism::planning::ActionBindingView action,
                                            const ConceptDenotations& denotations)
    {
        if (action.get_relation().get_name() != rule.get_action_name())
            return false;
        const auto objects = action.get_objects();
        if (objects.size() != denotations.size())
            return false;
        for (size_t i = 0; i < denotations.size(); ++i)
            if (!denotations[i].get().test(ygg::uint_t(objects[i].get_index())))
                return false;
        return true;
    }

    template<typename C>
    bool do_effects_match(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                          ProgramStateView<Kind> state,
                          const tyr::planning::Node<Kind>& node,
                          const tyr::planning::StateView<Kind>& target_state)
    {
        auto transition = m_environment.make_dl_transition_context(node.get_state(),
                                                                   target_state,
                                                                   state.get_module_state().get_arguments(),
                                                                   state.get_module_state().get_registers(),
                                                                   state.get_module_state().get_registers());
        return is_compatible_with(rule, transition);
    }

    template<typename C>
    bool do_rule_matches(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                         ProgramStateView<Kind> state,
                         const tyr::planning::Node<Kind>& node,
                         tyr::formalism::planning::ActionBindingView action,
                         const tyr::planning::StateView<Kind>& target_state)
    {
        if (!rule_is_applicable(rule, state, node))
            return false;
        const auto& denotations = evaluate_do_arguments(rule, state, node);
        return action_matches_do_arguments(rule, action, denotations) && do_effects_match(rule, state, node, target_state);
    }

    template<typename C>
    bool sketch_rule_matches_state(ygg::View<ygg::Index<Rule<SketchTag>>, C> rule,
                                   ProgramStateView<Kind> state,
                                   const tyr::planning::Node<Kind>& node,
                                   const tyr::planning::StateView<Kind>& target_state)
    {
        if (!has_current_source(rule, state))
            return false;
        auto transition = m_environment.make_dl_transition_context(node.get_state(),
                                                                   target_state,
                                                                   state.get_module_state().get_arguments(),
                                                                   state.get_module_state().get_registers(),
                                                                   state.get_module_state().get_registers());
        return is_compatible_with(rule, transition);
    }

    template<typename FeatureTag, typename C>
    static void append_call_argument(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> argument,
                                     runir::kr::dl::semantics::StateEvaluationContext<runir::kr::ExtFamilyTag, Kind>& context,
                                     ygg::Data<runir::kr::dl::semantics::CallArguments>& target)
    {
        const auto denotation = evaluate(argument, context);
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
            target.concept_arguments.push_back(denotation.get_index());
        else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
            target.role_arguments.push_back(denotation.get_index());
        else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
            target.boolean_arguments.push_back(denotation.get_index());
        else
            target.numerical_arguments.push_back(denotation.get_index());
    }

    template<typename C>
    auto evaluate_call_arguments(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, ProgramStateView<Kind> state, const tyr::planning::Node<Kind>& node)
    {
        auto result = checkout<runir::kr::dl::semantics::CallArguments>(m_task_context->dl_builder);
        auto state_context =
            m_environment.make_dl_context(node.get_state(), state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        rule.for_each_call_argument([&](auto argument) { append_call_argument(argument, state_context, *result); });
        return result;
    }

    static bool call_arguments_match_signature(ModuleView callee, const ygg::Data<runir::kr::dl::semantics::CallArguments>& arguments)
    {
        return arguments.concept_arguments.size() == callee.template get_arguments<runir::kr::dl::ConceptTag>().size()
               && arguments.role_arguments.size() == callee.template get_arguments<runir::kr::dl::RoleTag>().size()
               && arguments.boolean_arguments.size() == callee.template get_arguments<runir::kr::dl::BooleanTag>().size()
               && arguments.numerical_arguments.size() == callee.template get_arguments<runir::kr::dl::NumericalTag>().size();
    }

    template<BindingRuleKind RuleKindT, typename Value>
    auto
    bound_registers(RuleView<RuleKindT> rule, ProgramStateView<Kind> state, const Value& value, ygg::Data<runir::kr::dl::semantics::RegisterValues>& registers)
    {
        registers.concept_values = state.get_module_state().get_registers().get_data().concept_values;
        registers.role_values = state.get_module_state().get_registers().get_data().role_values;
        apply_binding(rule, value, registers);
        return get_or_create(*m_task_context->dl_denotation_repository, registers).first;
    }

    template<BindingRuleKind RuleKindT>
    bool binding_effects_match(RuleView<RuleKindT> rule,
                               ProgramStateView<Kind> state,
                               const tyr::planning::Node<Kind>& node,
                               runir::kr::dl::semantics::RegisterValuesView registers)
    {
        if (rule.get_effects().empty())
            return true;
        m_environment.get_dl_target_caches().clear(false);
        auto transition = m_environment.make_dl_transition_context(node.get_state(),
                                                                   node.get_state(),
                                                                   state.get_module_state().get_arguments(),
                                                                   state.get_module_state().get_registers(),
                                                                   registers);
        return is_compatible_with(rule, transition);
    }

    /// Bind a register and move memory while preserving the planning state and caller stack.
    template<runir::kr::dl::CategoryTag Category>
    Step choice_step(ProgramStateView<Kind> state, const detail::Choice<Category>& choice)
    {
        if (choice.exhausted())
        {
            auto failure = make_step(detail::ProgramOutcome::FAILURE, state);
            failure.rule = choice.rule;
            return failure;
        }
        const auto rule = choice.rule.get_variant().template get<ygg::Index<Rule<ChooseTag<Category>>>>();
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(m_task_context->dl_builder);
        auto target = state.get_module_state().get_data();
        ygg::set(bound_registers(rule, state, choice.current(), *registers), target.registers);
        ygg::set(rule.get_target(), target.memory_state);
        return applied(intern(target, state.get_call_stack()), choice.rule);
    }

    template<runir::kr::dl::CategoryTag Category, typename Emit, typename Stop>
    bool emit_rule(RuleView<LoadTag<Category>> rule,
                   RuleVariantView rule_variant,
                   ProgramStateView<Kind> state,
                   const tyr::planning::Node<Kind>& node,
                   Emit&& emit,
                   Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, node))
            return true;
        auto state_context =
            m_environment.make_dl_context(node.get_state(), state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        const auto denotation = evaluate(rule.get_feature(), state_context);
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(m_task_context->dl_builder);
        for (const auto value : denotation)
        {
            if (stop())
                return false;
            const auto target_registers = bound_registers(rule, state, value, *registers);
            if (!binding_effects_match(rule, state, node, target_registers))
                continue;
            auto target = state.get_module_state().get_data();
            ygg::set(target_registers, target.registers);
            ygg::set(rule.get_target(), target.memory_state);
            if (!emit(applied(intern(target, state.get_call_stack()), rule_variant)))
                return false;
        }
        return true;
    }

    template<runir::kr::dl::CategoryTag Category, typename Emit, typename Stop>
    bool emit_rule(RuleView<ChooseTag<Category>> rule,
                   RuleVariantView rule_variant,
                   ProgramStateView<Kind> state,
                   const tyr::planning::Node<Kind>& node,
                   Emit&& emit,
                   Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, node))
            return true;
        auto state_context =
            m_environment.make_dl_context(node.get_state(), state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        const auto denotation = evaluate(rule.get_feature(), state_context);
        if (stop())
            return false;
        if (rule.get_effects().empty())
            return emit(detail::Choice<Category>(rule_variant, denotation));

        auto admitted = m_task_context->dl_builder.template get_builder<runir::kr::dl::semantics::Denotation<Category>>(denotation.get_data().num_objects);
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(m_task_context->dl_builder);
        for (const auto value : denotation)
        {
            if (stop())
                return false;
            const auto target_registers = bound_registers(rule, state, value, *registers);
            if (!binding_effects_match(rule, state, node, target_registers))
                continue;
            if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                admitted->get().set(ygg::uint_t(value.get_index()));
            else
                admitted->get(value.first.get_index()).set(ygg::uint_t(value.second.get_index()));
        }
        if (stop())
            return false;
        return emit(detail::Choice<Category>(rule_variant, runir::kr::dl::semantics::detail::materialize_denotation(admitted, state_context).first));
    }

    LabeledNode successor(const tyr::planning::Node<Kind>& node, tyr::formalism::planning::ActionBindingView binding)
    {
        auto& search = *m_task_context->search_context;
        return { binding, search.successor_generator->get_successor_node(node, binding, *search.state_repository, *search.axiom_evaluator) };
    }

    template<typename Emit, typename Stop>
    bool
    emit_rule(RuleView<DoTag> rule, RuleVariantView rule_variant, ProgramStateView<Kind> state, const tyr::planning::Node<Kind>& node, Emit&& emit, Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, node))
            return true;
        const auto& denotations = evaluate_do_arguments(rule, state, node);
        if (std::ranges::any_of(denotations, [](const auto& denotation) { return denotation.get().count() == 0; }))
            return true;
        auto& search = *m_task_context->search_context;
        for (const auto action : search.task->get_task().get_domain().get_actions())
        {
            if (action.get_name().str() != rule.get_action_name())
                continue;
            const auto visit = [&](tyr::formalism::planning::ActionBindingView binding)
            {
                if (stop())
                    return false;
                if (!action_matches_do_arguments(rule, binding, denotations))
                    return true;
                const auto candidate = successor(node, binding);
                m_environment.get_dl_target_caches().clear(false);
                if (!do_effects_match(rule, state, node, candidate.node.get_state()))
                    return true;
                return emit(planning_step(state, candidate, rule_variant, rule.get_target()));
            };
            return search.successor_generator->for_each_applicable_action_binding(node, action, std::ref(visit));
        }
        return true;
    }

    void check_action_effects(RuleView<ActionTag> rule,
                              ProgramStateView<Kind> state,
                              const tyr::planning::Node<Kind>& node,
                              const LabeledNode& candidate,
                              std::span<const ygg::uint_t> tuple)
    {
        m_environment.get_dl_target_caches().clear(false);
        auto transition = m_environment.make_dl_transition_context(node.get_state(),
                                                                   candidate.node.get_state(),
                                                                   state.get_module_state().get_arguments(),
                                                                   state.get_module_state().get_registers(),
                                                                   state.get_module_state().get_registers());
        for (const auto effect : rule.get_effects())
            if (!is_compatible_with(effect, transition))
                detail::action_rule_contract_error(rule, node.get_state(), tuple, "offered transition violates declared effects");
    }

    template<typename Emit, typename Stop>
    bool emit_rule(RuleView<ActionTag> rule,
                   RuleVariantView rule_variant,
                   ProgramStateView<Kind> state,
                   const tyr::planning::Node<Kind>& node,
                   Emit&& emit,
                   Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, node))
            return true;
        auto state_context =
            m_environment.make_dl_context(node.get_state(), state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        const auto query = evaluate(rule.get_query_feature(), state_context);
        m_action_rule_evaluator.action(rule, node.get_state(), query.arity());
        for (std::size_t i = 0; i < query.size(); ++i)
        {
            if (stop())
                return false;
            const auto tuple = query[i];
            const auto binding = m_action_rule_evaluator.applicable_binding(rule, node, tuple);
            const auto candidate = successor(node, binding);
            check_action_effects(rule, state, node, candidate, tuple);
            if (!emit(planning_step(state, candidate, rule_variant, rule.get_target())))
                return false;
        }
        return true;
    }

    template<typename Emit, typename Stop>
    bool emit_rule(RuleView<SketchTag> rule,
                   RuleVariantView rule_variant,
                   ProgramStateView<Kind> state,
                   const tyr::planning::Node<Kind>& node,
                   Emit&& emit,
                   Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, node))
            return true;
        if (rule.get_effects().empty())
        {
            if (stop())
                return false;
            auto target = state.get_module_state().get_data();
            ygg::set(rule.get_target(), target.memory_state);
            return emit(applied(intern(target, state.get_call_stack()), rule_variant));
        }
        auto& generator = *m_task_context->search_context->successor_generator;
        const auto visit = [&](tyr::formalism::planning::ActionBindingView binding)
        {
            if (stop())
                return false;
            const auto candidate = successor(node, binding);
            m_environment.get_dl_target_caches().clear(false);
            if (!sketch_rule_matches_state(rule, state, node, candidate.node.get_state()))
                return true;
            return emit(planning_step(state, candidate, rule_variant, rule.get_target()));
        };
        return generator.for_each_applicable_action_binding(node, std::ref(visit));
    }

    template<typename Emit, typename Stop>
    bool emit_rule(RuleView<CallTag> rule,
                   RuleVariantView rule_variant,
                   ProgramStateView<Kind> state,
                   const tyr::planning::Node<Kind>& node,
                   Emit&& emit,
                   Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, node))
            return true;
        auto arguments = evaluate_call_arguments(rule, state, node);
        const auto callee = m_program.find_module(rule.get_callee().get_index());
        if (stop())
            return false;
        if (!callee || !call_arguments_match_signature(*callee, *arguments))
            return emit(make_step(detail::ProgramOutcome::MALFORMED_CALL, state));

        auto target = state.get_module_state().get_data();
        auto caller = ygg::Data<CallStack>(target.module, rule.get_target().get_index(), target.registers, target.arguments, state.get_data().call_stack);
        ygg::set(*callee, target.module);
        ygg::set(callee->get_entry_memory_state(), target.memory_state);
        ygg::set(empty_registers(*callee), target.registers);
        ygg::set(get_or_create(*m_task_context->dl_denotation_repository, *arguments).first, target.arguments);
        return emit(applied(intern(target, get_or_create(*m_task_context->execution_repository, caller).first), rule_variant));
    }

    template<BindingRuleKind RuleKindT>
    bool matches(RuleView<RuleKindT>, ProgramStateView<Kind>, const tyr::planning::Node<Kind>&, const LabeledNode&)
    {
        return false;
    }

    bool matches(RuleView<CallTag>, ProgramStateView<Kind>, const tyr::planning::Node<Kind>&, const LabeledNode&) { return false; }

    bool matches(RuleView<DoTag> rule, ProgramStateView<Kind> state, const tyr::planning::Node<Kind>& node, const LabeledNode& candidate)
    {
        return do_rule_matches(rule, state, node, candidate.label, candidate.node.get_state());
    }

    bool matches(RuleView<SketchTag> rule, ProgramStateView<Kind> state, const tyr::planning::Node<Kind>& node, const LabeledNode& candidate)
    {
        return !rule.get_effects().empty() && sketch_rule_matches_state(rule, state, node, candidate.node.get_state());
    }

    bool matches(RuleView<ActionTag> rule, ProgramStateView<Kind> state, const tyr::planning::Node<Kind>& node, const LabeledNode& candidate)
    {
        if (!rule_is_applicable(rule, state, node) || candidate.label.get_relation().get_name() != rule.get_action_name())
            return false;
        auto state_context =
            m_environment.make_dl_context(node.get_state(), state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        const auto query = evaluate(rule.get_query_feature(), state_context);
        m_action_rule_evaluator.action(rule, node.get_state(), query.arity());
        m_action_tuple.clear();
        for (const auto object : candidate.label.get_objects())
            m_action_tuple.push_back(ygg::uint_t(object.get_index()));
        if (m_action_tuple.size() != query.arity() || !query.contains(std::span<const ygg::uint_t>(m_action_tuple)))
            return false;
        m_action_rule_evaluator.applicable_binding(rule, node, m_action_tuple);
        check_action_effects(rule, state, node, candidate, m_action_tuple);
        return true;
    }

    template<RuleKind RuleKindT>
        requires(BindingRuleKind<RuleKindT> || std::same_as<RuleKindT, CallTag>)
    std::optional<Step> apply_rule(RuleView<RuleKindT> rule,
                                   RuleVariantView rule_variant,
                                   ProgramStateView<Kind> state,
                                   const tyr::planning::Node<Kind>& node,
                                   const std::optional<LabeledNode>&)
    {
        auto result = std::optional<Expansion> {};
        emit_rule(
            rule,
            rule_variant,
            state,
            node,
            [&](Expansion expansion)
            {
                result = std::move(expansion);
                return false;
            },
            [] { return false; });
        if (!result)
            return std::nullopt;
        if constexpr (ChooseRuleView<decltype(rule)>)
            return choice_step(state, std::get<detail::Choice<typename RuleKindT::Category>>(*result));
        else
            return std::get<Step>(std::move(*result));
    }

    template<typename Tag>
        requires(std::same_as<Tag, DoTag> || std::same_as<Tag, ActionTag> || std::same_as<Tag, SketchTag>)
    std::optional<Step> apply_rule(RuleView<Tag> rule,
                                   RuleVariantView rule_variant,
                                   ProgramStateView<Kind> state,
                                   const tyr::planning::Node<Kind>& node,
                                   const std::optional<LabeledNode>& candidate)
    {
        if constexpr (std::same_as<Tag, SketchTag>)
            if (rule.get_effects().empty())
            {
                if (!rule_is_applicable(rule, state, node))
                    return std::nullopt;
                auto target = state.get_module_state().get_data();
                ygg::set(rule.get_target(), target.memory_state);
                return applied(intern(target, state.get_call_stack()), rule_variant);
            }
        if (!candidate || !matches(rule, state, node, *candidate))
            return std::nullopt;
        return planning_step(state, *candidate, rule_variant, rule.get_target());
    }

    Step make_step(detail::ProgramOutcome status, ProgramStateView<Kind> state) { return Step(status, state, m_task_context); }

    /// With no emitted rule outcome, return to the caller or report an open top-level state.
    /// Restore caller control and bindings while retaining the planning state reached by the callee.
    Step fallback(ProgramStateView<Kind> state)
    {
        if (const auto caller = state.get_call_stack())
        {
            auto target = state.get_module_state().get_data();
            const auto& saved = caller->get_data();
            target.module = saved.module;
            target.memory_state = saved.return_memory_state;
            target.registers = saved.registers;
            target.arguments = saved.arguments;
            return make_step(detail::ProgramOutcome::RESTORED_CALLER, intern(target, caller->get_caller()));
        }
        return make_step(detail::ProgramOutcome::NO_APPLICABLE_ACTION, state);
    }

    Step applied(ProgramStateView<Kind> state, RuleVariantView rule)
    {
        auto step = make_step(detail::ProgramOutcome::APPLIED, state);
        step.rule = rule;
        return step;
    }

    Step planning_step(ProgramStateView<Kind> state, const LabeledNode& successor, RuleVariantView rule, MemoryStateView memory_state)
    {
        auto target = state.get_module_state().get_data();
        ygg::set(successor.node.get_state(), target.state);
        ygg::set(memory_state, target.memory_state);
        auto step = applied(intern(target, state.get_call_stack()), rule);
        step.planning_successor = successor.pack();
        step.state_transition = datasets::StateGraphEdgeLabel { successor.label, ygg::float_t(1) };
        return step;
    }

    runir::kr::TaskContextPtr<Kind> m_task_context;
    ProgramView m_program;
    EvaluationEnvironment<Kind> m_environment;
    detail::ActionRuleEvaluator<Kind> m_action_rule_evaluator;
    std::vector<ygg::uint_t> m_action_tuple;
};

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class SuccessorExpander<tyr::GroundTag>;
extern template class SuccessorExpander<tyr::LiftedTag>;

#endif

}  // namespace runir::kr::ps::ext

#endif
