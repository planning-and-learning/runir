#ifndef RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_

// Single source of module-program execution steps. Choose bindings are alternative
// continuations; ordinary successors are checked universally or selected greedily.

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/action_rule.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"
#include "runir/kr/ps/ext/expansion_policy.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <cassert>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/dependent_false.hpp>

namespace runir::kr::ps::ext
{

template<tyr::TaskKind Kind>
class SuccessorExpander
{
private:
    struct Frame
    {
        tyr::planning::StateView<Kind> state;
        ModuleView module;
        MemoryStateView memory_state;
        runir::kr::dl::semantics::RegisterValuesView registers;
        runir::kr::dl::semantics::CallArgumentsView arguments;
        std::optional<CallStackView<Kind>> caller;
    };

    enum class RuleExecutionStatus
    {
        APPLIED,
        NOT_APPLICABLE,
        MALFORMED_CALL
    };

public:
    using LabeledNode = tyr::planning::LabeledNode<Kind>;
    using Step = detail::ProgramStep<Kind>;

    SuccessorExpander(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program) :
        m_task_context(task_context ? std::move(task_context) : throw std::invalid_argument("SuccessorExpander requires a task context.")),
        m_program(program),
        m_goal_strategy(*m_task_context->search_context->task),
        m_initial_state(m_task_context->search_context->successor_generator
                            ->get_initial_node(*m_task_context->search_context->state_repository, *m_task_context->search_context->axiom_evaluator)
                            .get_state()),
        m_static_goal_satisfied(m_goal_strategy.is_static_goal_satisfied(*m_task_context->search_context->task)),
        m_environment(*m_task_context, m_program),
        m_action_rule_evaluator(m_task_context->search_context->task)
    {
        if (&m_program.get_context() != m_task_context->domain_context->ext_repository.get())
            throw std::invalid_argument("SuccessorExpander requires a program from the domain context repository.");
    }

    const auto& get_task_context() const noexcept { return m_task_context; }

    ProgramStateView<Kind> initial_state()
    {
        auto arguments = checkout<runir::kr::dl::semantics::CallArguments>(m_task_context->dl_builder);
        const auto module_ = m_program.get_entry_module();
        const auto frame = Frame { m_initial_state,
                                   module_,
                                   module_.get_entry_memory_state(),
                                   empty_registers(module_),
                                   get_or_create(*m_task_context->dl_denotation_repository, *arguments).first,
                                   std::nullopt };
        return intern(frame);
    }

    bool is_goal(const tyr::planning::StateView<Kind>& state)
    {
        return m_static_goal_satisfied && m_goal_strategy.is_dynamic_goal_satisfied(m_initial_state, state);
    }

    bool is_goal(ProgramStateView<Kind> state) { return is_goal(state.get_state()); }

    std::vector<Step> load_steps(ProgramStateView<Kind> state)
    {
        auto result = std::vector<Step> {};
        load_steps(std::move(state), result);
        return result;
    }

    void load_steps(ProgramStateView<Kind> state, std::vector<Step>& out_steps)
    {
        load_steps_until(std::move(state), [] { return false; }, out_steps);
    }

    std::vector<Step> load_steps_until(ProgramStateView<Kind> state, auto&& stop)
    {
        auto result = std::vector<Step> {};
        load_steps_until(std::move(state), stop, result);
        return result;
    }

    void load_steps_until(ProgramStateView<Kind> state, auto&& stop, std::vector<Step>& out_steps)
    {
        auto context = materialize(state);
        collect_steps<EagerExpansionPolicy, LoadTag<runir::kr::dl::ConceptTag>, LoadTag<runir::kr::dl::RoleTag>>(context, generated_successors(), stop, out_steps);
    }

    std::vector<Step> choose_steps(ProgramStateView<Kind> state)
    {
        auto result = std::vector<Step> {};
        choose_steps(std::move(state), result);
        return result;
    }

    void choose_steps(ProgramStateView<Kind> state, std::vector<Step>& out_steps)
    {
        choose_steps_until(std::move(state), [] { return false; }, out_steps);
    }

    std::vector<Step> choose_steps_until(ProgramStateView<Kind> state, auto&& stop)
    {
        auto result = std::vector<Step> {};
        choose_steps_until(std::move(state), stop, result);
        return result;
    }

    void choose_steps_until(ProgramStateView<Kind> state, auto&& stop, std::vector<Step>& out_steps)
    {
        auto context = materialize(state);
        collect_steps<EagerExpansionPolicy, ChooseTag<runir::kr::dl::ConceptTag>, ChooseTag<runir::kr::dl::RoleTag>>(context, generated_successors(), stop, out_steps);
    }

    std::vector<LabeledNode> labeled_successors(ProgramStateView<Kind> state)
    {
        auto result = std::vector<LabeledNode> {};
        labeled_successors(std::move(state), result);
        return result;
    }

    void labeled_successors(ProgramStateView<Kind> state, std::vector<LabeledNode>& out_successors)
    {
        auto& search_context = *m_task_context->search_context;
        auto& successor_generator = *search_context.successor_generator;
        const auto node = successor_generator.get_node(*search_context.state_repository, state.get_state().get_index());
        successor_generator.get_labeled_successor_nodes(node, *search_context.state_repository, *search_context.axiom_evaluator, out_successors);
    }

    std::vector<Step> control_steps(ProgramStateView<Kind> state, const std::vector<LabeledNode>& successors)
    {
        auto result = std::vector<Step> {};
        control_steps(std::move(state), successors, result);
        return result;
    }

    void control_steps(ProgramStateView<Kind> state, const std::vector<LabeledNode>& successors, std::vector<Step>& out_steps)
    {
        control_steps_until(std::move(state), successors, [] { return false; }, out_steps);
    }

    std::vector<Step> control_steps_until(ProgramStateView<Kind> state, const std::vector<LabeledNode>& successors, auto&& stop)
    {
        auto result = std::vector<Step> {};
        control_steps_until(std::move(state), successors, stop, result);
        return result;
    }

    void control_steps_until(ProgramStateView<Kind> state, const std::vector<LabeledNode>& successors, auto&& stop, std::vector<Step>& out_steps)
    {
        auto context = materialize(state);
        collect_steps<EagerExpansionPolicy, DoTag, ActionTag, CallTag, SketchTag>(context, supplied_successors(successors), stop, out_steps);
        if (!stop() && out_steps.empty())
            out_steps.push_back(fallback(std::move(context)));
    }

    std::vector<Step> control_steps(ProgramStateView<Kind> state)
    {
        auto result = std::vector<Step> {};
        control_steps(std::move(state), result);
        return result;
    }

    void control_steps(ProgramStateView<Kind> state, std::vector<Step>& out_steps)
    {
        auto context = materialize(state);
        collect_steps<EagerExpansionPolicy, DoTag, ActionTag, CallTag, SketchTag>(context, generated_successors(), [] { return false; }, out_steps);
        if (out_steps.empty())
            out_steps.push_back(fallback(std::move(context)));
    }

    std::vector<Step> steps(ProgramStateView<Kind> state)
    {
        auto result = std::vector<Step> {};
        steps(std::move(state), result);
        return result;
    }

    void steps(ProgramStateView<Kind> state, std::vector<Step>& out_steps)
    {
        steps_until(std::move(state), [] { return false; }, out_steps);
    }

    std::vector<Step> steps_until(ProgramStateView<Kind> state, auto&& stop)
    {
        auto result = std::vector<Step> {};
        steps_until(std::move(state), stop, result);
        return result;
    }

    template<ExpansionPolicy Policy = EagerExpansionPolicy>
    void steps_until(ProgramStateView<Kind> state, auto&& stop, std::vector<Step>& out_steps)
    {
        auto context = materialize(state);
        collect_steps<Policy, LoadTag<runir::kr::dl::ConceptTag>, LoadTag<runir::kr::dl::RoleTag>, ChooseTag<runir::kr::dl::ConceptTag>,
                      ChooseTag<runir::kr::dl::RoleTag>, DoTag, ActionTag, CallTag, SketchTag>(context, generated_successors(), stop, out_steps);
        if (!stop() && out_steps.empty())
            out_steps.push_back(fallback(std::move(context)));
    }

    std::vector<Step> steps(ProgramStateView<Kind> state, const std::vector<LabeledNode>& successors)
    {
        auto result = std::vector<Step> {};
        steps(std::move(state), successors, result);
        return result;
    }

    void steps(ProgramStateView<Kind> state, const std::vector<LabeledNode>& successors, std::vector<Step>& out_steps)
    {
        steps_until(std::move(state), successors, [] { return false; }, out_steps);
    }

    std::vector<Step> steps_until(ProgramStateView<Kind> state, const std::vector<LabeledNode>& successors, auto&& stop)
    {
        auto result = std::vector<Step> {};
        steps_until(std::move(state), successors, stop, result);
        return result;
    }

    template<ExpansionPolicy Policy = EagerExpansionPolicy>
    void steps_until(ProgramStateView<Kind> state, const std::vector<LabeledNode>& successors, auto&& stop, std::vector<Step>& out_steps)
    {
        auto context = materialize(state);
        collect_steps<Policy, LoadTag<runir::kr::dl::ConceptTag>, LoadTag<runir::kr::dl::RoleTag>, ChooseTag<runir::kr::dl::ConceptTag>,
                      ChooseTag<runir::kr::dl::RoleTag>, DoTag, ActionTag, CallTag, SketchTag>(context, supplied_successors(successors), stop, out_steps);
        if (!stop() && out_steps.empty())
            out_steps.push_back(fallback(std::move(context)));
    }

    std::optional<RuleVariantView>
    matching_rule(ProgramStateView<Kind> state, tyr::formalism::planning::ActionBindingView action, tyr::planning::StateView<Kind> target_state)
    {
        auto context = materialize(state);
        return matching_rule_for_candidate(context, LabeledNode { action, tyr::planning::Node<Kind>(std::move(target_state), ygg::float_t(0)) });
    }

    std::optional<Step> apply(ProgramStateView<Kind> state,
                              RuleVariantView rule,
                              std::optional<tyr::formalism::planning::ActionBindingView> action = std::nullopt,
                              std::optional<tyr::planning::StateView<Kind>> target_state = std::nullopt)
    {
        auto context = materialize(state);
        std::optional<LabeledNode> candidate;
        if (action && target_state)
            candidate = LabeledNode { *action, tyr::planning::Node<Kind>(*target_state, ygg::float_t(0)) };
        return apply_rule(context, rule, std::move(candidate));
    }

private:
    Frame materialize(ProgramStateView<Kind> state) const
    {
        assert(&state.get_context() == m_task_context->execution_repository.get());
        const auto program = state.get_program();
        if (&program.get_context() != &m_program.get_context() || program.get_index() != m_program.get_index())
            throw std::invalid_argument("SuccessorExpander requires an execution state from the selected program.");
        const auto module_state = state.get_module_state();
        return Frame { module_state.get_state(),     module_state.get_module(),    module_state.get_memory_state(),
                       module_state.get_registers(), module_state.get_arguments(), state.get_call_stack() };
    }

    runir::kr::dl::semantics::RegisterValuesView empty_registers(ModuleView module_)
    {
        auto data = checkout<runir::kr::dl::semantics::RegisterValues>(m_task_context->dl_builder);
        data->concept_values.resize(module_.template get_registers<runir::kr::dl::ConceptTag>().size());
        data->role_values.resize(module_.template get_registers<runir::kr::dl::RoleTag>().size());
        return get_or_create(*m_task_context->dl_denotation_repository, *data).first;
    }

    CallStackView<Kind> intern_call_stack(const Frame& frame, MemoryStateView return_memory_state)
    {
        auto data = checkout<CallStack>(m_task_context->execution_builder);
        ygg::set(frame.module, data->module);
        ygg::set(return_memory_state, data->return_memory_state);
        ygg::set(frame.registers, data->registers);
        ygg::set(frame.arguments, data->arguments);
        ygg::set(frame.caller, data->caller);
        return get_or_create(*m_task_context->execution_repository, *data).first;
    }

    ProgramStateView<Kind> intern(const Frame& frame)
    {
        auto module_state = checkout<ModuleState<Kind>>(m_task_context->execution_builder);
        ygg::set(frame.state, module_state->state);
        ygg::set(frame.module, module_state->module);
        ygg::set(frame.memory_state, module_state->memory_state);
        ygg::set(frame.registers, module_state->registers);
        ygg::set(frame.arguments, module_state->arguments);

        auto data = checkout<ProgramState<Kind>>(m_task_context->execution_builder);
        ygg::set(m_program, data->program);
        ygg::set(get_or_create(*m_task_context->execution_repository, *module_state).first, data->module_state);
        ygg::set(frame.caller, data->call_stack);
        return get_or_create(*m_task_context->execution_repository, *data).first;
    }

    void enter_module(Frame& frame, ModuleView module_, MemoryStateView return_memory_state, runir::kr::dl::semantics::CallArgumentsView arguments)
    {
        frame.caller = intern_call_stack(frame, return_memory_state);
        frame.module = module_;
        frame.memory_state = module_.get_entry_memory_state();
        frame.registers = empty_registers(module_);
        frame.arguments = arguments;
    }

    bool restore_caller(Frame& frame)
    {
        if (!frame.caller)
            return false;
        const auto caller = *frame.caller;
        frame.module = caller.get_module();
        frame.memory_state = caller.get_return_memory_state();
        frame.registers = caller.get_registers();
        frame.arguments = caller.get_arguments();
        frame.caller = caller.get_caller();
        return true;
    }

    template<RuleKind RuleKindT, typename C>
    static bool has_current_source(ygg::View<ygg::Index<Rule<RuleKindT>>, C> rule, const Frame& frame)
    {
        return rule.get_source().get_index() == frame.memory_state.get_index();
    }

    template<RuleKind RuleKindT, typename C>
    bool conditions_are_compatible(ygg::View<ygg::Index<Rule<RuleKindT>>, C> rule, const Frame& frame)
    {
        auto state_context = m_environment.make_dl_context(frame.state, frame.arguments, frame.registers);
        return runir::kr::ps::ext::conditions_are_compatible(rule, state_context);
    }

    template<RuleKind RuleKindT, typename C>
    bool rule_is_applicable(ygg::View<ygg::Index<Rule<RuleKindT>>, C> rule, const Frame& frame)
    {
        return has_current_source(rule, frame) && conditions_are_compatible(rule, frame);
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
    auto& evaluate_do_arguments(ygg::View<ygg::Index<Rule<DoTag>>, C> rule, const Frame& frame)
    {
        const auto arguments = rule.get_action_arguments();
        auto& denotations = m_environment.prepare_do_argument_denotations(arguments.size());
        auto state_context = m_environment.make_dl_context(frame.state, frame.arguments, frame.registers);
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

    template<typename ConceptDenotations, typename C>
    bool do_successor_matches(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                              const Frame& frame,
                              const ConceptDenotations& denotations,
                              tyr::formalism::planning::ActionBindingView action,
                              const tyr::planning::StateView<Kind>& target_state)
    {
        if (!action_matches_do_arguments(rule, action, denotations))
            return false;
        auto transition = m_environment.make_dl_transition_context(frame.state,
                                                                   target_state,
                                                                   frame.arguments,
                                                                   frame.registers,
                                                                   frame.registers);
        return is_compatible_with(rule, transition);
    }

    template<typename C>
    bool do_rule_matches(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                         const Frame& frame,
                         tyr::formalism::planning::ActionBindingView action,
                         const tyr::planning::StateView<Kind>& target_state)
    {
        if (!rule_is_applicable(rule, frame))
            return false;
        const auto& denotations = evaluate_do_arguments(rule, frame);
        return do_successor_matches(rule, frame, denotations, action, target_state);
    }

    template<typename C>
    static void apply_do_successor(ygg::View<ygg::Index<Rule<DoTag>>, C> rule, const LabeledNode& successor, Frame& frame)
    {
        frame.state = successor.node.get_state();
        frame.memory_state = rule.get_target();
    }

    template<typename C>
    bool sketch_rule_matches_state(ygg::View<ygg::Index<Rule<SketchTag>>, C> rule,
                                   const Frame& frame,
                                   const tyr::planning::StateView<Kind>& target_state)
    {
        if (!has_current_source(rule, frame))
            return false;
        auto transition = m_environment.make_dl_transition_context(frame.state,
                                                                   target_state,
                                                                   frame.arguments,
                                                                   frame.registers,
                                                                   frame.registers);
        return is_compatible_with(rule, transition);
    }

    template<typename C>
    RuleExecutionStatus execute_sketch(ygg::View<ygg::Index<Rule<SketchTag>>, C> rule,
                                       Frame& frame,
                                       const std::vector<LabeledNode>& successors)
    {
        if (!has_current_source(rule, frame))
            return RuleExecutionStatus::NOT_APPLICABLE;
        if (rule.get_effects().empty() && conditions_are_compatible(rule, frame))
        {
            frame.memory_state = rule.get_target();
            return RuleExecutionStatus::APPLIED;
        }
        for (const auto& successor : successors)
        {
            m_environment.get_dl_target_caches().clear(false);
            if (sketch_rule_matches_state(rule, frame, successor.node.get_state()))
            {
                frame.state = successor.node.get_state();
                frame.memory_state = rule.get_target();
                return RuleExecutionStatus::APPLIED;
            }
        }
        return RuleExecutionStatus::NOT_APPLICABLE;
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
    auto evaluate_call_arguments(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, const Frame& frame)
    {
        auto result = checkout<runir::kr::dl::semantics::CallArguments>(m_task_context->dl_builder);
        auto state_context = m_environment.make_dl_context(frame.state, frame.arguments, frame.registers);
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

    template<typename C>
    RuleExecutionStatus execute_call(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, Frame& frame)
    {
        if (!rule_is_applicable(rule, frame))
            return RuleExecutionStatus::NOT_APPLICABLE;
        auto arguments = evaluate_call_arguments(rule, frame);
        const auto callee = m_program.find_module(rule.get_callee().get_index());
        if (!callee || !call_arguments_match_signature(*callee, *arguments))
            return RuleExecutionStatus::MALFORMED_CALL;
        enter_module(frame, *callee, rule.get_target(), get_or_create(*m_task_context->dl_denotation_repository, *arguments).first);
        return RuleExecutionStatus::APPLIED;
    }

    static auto supplied_successors(const std::vector<LabeledNode>& successors)
    {
        return [&successors](auto&& emit, auto&& stop, auto&&...)
        {
            for (const auto& successor : successors)
            {
                if (stop())
                    return;
                emit(successor);
            }
        };
    }

    auto generated_successors()
    {
        m_all_successors_ready = false;
        m_all_successors.clear();
        return [this](auto&& emit, auto&& stop, auto rule, auto&&... arguments)
        { for_each_successor(rule, std::forward<decltype(arguments)>(arguments)..., emit, stop); };
    }

    void for_each_cached_successor(auto&& emit, auto&& stop)
    {
        for (const auto& successor : m_all_successors)
        {
            if (stop())
                return;
            emit(successor.unpack());
        }
    }

    void for_each_successor(RuleView<DoTag> rule, const Frame& context, const auto& denotations, auto&& emit, auto&& stop)
    {
        if (stop() || std::ranges::any_of(denotations, [](const auto& denotation) { return denotation.get().count() == 0; }))
            return;
        if (m_all_successors_ready)
        {
            for_each_cached_successor(emit, stop);
            return;
        }

        auto& search_context = *m_task_context->search_context;
        auto& generator = *search_context.successor_generator;
        for (const auto action : search_context.task->get_task().get_domain().get_actions())
        {
            if (action.get_name().str() != rule.get_action_name())
                continue;
            const auto node = generator.get_node(*search_context.state_repository, context.state.get_index());
            generator.for_each_applicable_action_binding(node, action, [&](auto binding)
            {
                if (stop())
                    return false;
                if (action_matches_do_arguments(rule, binding, denotations))
                    emit(LabeledNode { binding, generator.get_successor_node(node, binding, *search_context.state_repository, *search_context.axiom_evaluator) });
                return !stop();
            });
            return;
        }
    }

    void for_each_successor(RuleView<SketchTag>, const Frame& context, auto&& emit, auto&& stop)
    {
        if (stop())
            return;
        if (m_all_successors_ready)
        {
            for_each_cached_successor(emit, stop);
            return;
        }
        auto& search_context = *m_task_context->search_context;
        auto& generator = *search_context.successor_generator;
        const auto node = generator.get_node(*search_context.state_repository, context.state.get_index());
        m_all_successors_ready = generator.for_each_labeled_successor_node(node, *search_context.state_repository, *search_context.axiom_evaluator,
            [&](LabeledNode successor)
            {
                if (stop())
                    return false;
                m_all_successors.push_back(successor.pack());
                emit(successor);
                return !stop();
            });
    }

    void for_each_successor(RuleView<ActionTag> rule,
                            const Frame& context,
                            ygg::database::RelationView<> query,
                            auto&& emit,
                            auto&& stop)
    {
        if (stop() || query.empty())
            return;
        auto& search_context = *m_task_context->search_context;
        auto& generator = *search_context.successor_generator;
        const auto node = generator.get_node(*search_context.state_repository, context.state.get_index());
        for (size_t i = 0; i < query.size(); ++i)
        {
            if (stop())
                return;
            const auto binding = m_action_rule_evaluator.applicable_binding(rule, node, query[i]);
            emit(LabeledNode { binding, generator.get_successor_node(node, binding, *search_context.state_repository, *search_context.axiom_evaluator) }, true);
        }
    }

    bool action_successor_matches(RuleView<ActionTag> rule,
                                  const Frame& context,
                                  ygg::database::RelationView<> query,
                                  const LabeledNode& candidate,
                                  bool binding_checked = false)
    {
        if (candidate.label.get_relation().get_name() != rule.get_action_name())
            return false;
        m_action_tuple.clear();
        for (const auto object : candidate.label.get_objects())
            m_action_tuple.push_back(ygg::uint_t(object.get_index()));
        if (m_action_tuple.size() != query.arity() || !query.contains(std::span<const ygg::uint_t>(m_action_tuple)))
            return false;
        if (!binding_checked)
        {
            auto& search_context = *m_task_context->search_context;
            const auto node = search_context.successor_generator->get_node(*search_context.state_repository, context.state.get_index());
            m_action_rule_evaluator.applicable_binding(rule, node, m_action_tuple);
        }
        auto transition = m_environment.make_dl_transition_context(context.state,
                                                                   candidate.node.get_state(),
                                                                   context.arguments,
                                                                   context.registers,
                                                                   context.registers);
        for (const auto effect : rule.get_effects())
            if (!is_compatible_with(effect, transition))
                detail::action_rule_contract_error(rule, context.state, m_action_tuple, "offered transition violates declared effects");
        return true;
    }

    std::optional<RuleVariantView> matching_rule_for_candidate(const Frame& context, const LabeledNode& candidate)
    {
        m_environment.get_dl_caches().clear(false);
        m_environment.get_dl_target_caches().clear(false);
        for (const auto& transition : context.module.get_memory_transitions())
            for (auto rule : transition)
                if (selects(rule, context, candidate))
                    return rule;
        return std::nullopt;
    }

    std::optional<Step> apply_rule(const Frame& context, RuleVariantView rule, std::optional<LabeledNode> candidate = std::nullopt)
    {
        m_environment.get_dl_caches().clear(false);
        const auto successors = candidate ? std::vector<LabeledNode> { *candidate } : std::vector<LabeledNode> {};
        auto steps = std::vector<Step> {};
        const auto stop = [] { return false; };
        ygg::visit([&](auto concrete) { append_steps<EagerExpansionPolicy>(concrete, rule, context, supplied_successors(successors), steps, stop); }, rule.get_variant());
        return steps.empty() ? std::nullopt : std::optional(std::move(steps.front()));
    }

    template<ExpansionPolicy Policy, typename... Kinds>
    void collect_steps(const Frame& context, auto&& visit_successors, auto&& stop, std::vector<Step>& out_steps)
    {
        m_environment.get_dl_caches().clear(false);
        out_steps.clear();
        for (const auto& transition : context.module.get_memory_transitions())
        {
            for (auto rule_variant : transition)
            {
                if (stop())
                    return;
                ygg::visit(
                    [&](auto rule)
                    {
                        using R = std::decay_t<decltype(rule)>;
                        if constexpr ((std::same_as<R, RuleView<Kinds>> || ...))
                        {
                            const auto initial_size = out_steps.size();
                            const auto rule_stop = [&]
                            {
                                if constexpr (std::same_as<Policy, LazyExpansionPolicy> && !ChooseRuleView<R>)
                                    return stop() || out_steps.size() != initial_size;
                                else
                                    return stop();
                            };
                            append_steps<Policy>(rule, rule_variant, context, visit_successors, out_steps, rule_stop);
                        }
                    },
                    rule_variant.get_variant());
                if constexpr (std::same_as<Policy, LazyExpansionPolicy>)
                    if (!out_steps.empty())
                        return;
            }
        }
    }

    template<ExpansionPolicy Policy, typename R>
    void append_steps(R rule,
                      RuleVariantView rule_variant,
                      const Frame& context,
                      auto&& visit_successors,
                      std::vector<Step>& result,
                      auto&& stop)
    {
        if constexpr (BindingRuleView<R>)
        {
            if (!rule_is_applicable(rule, context))
                return;

            const auto initial_size = result.size();
            auto state_context = m_environment.make_dl_context(context.state, context.arguments, context.registers);
            const auto denotation = evaluate(rule.get_feature(), state_context);
            auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(m_task_context->dl_builder);
            for (const auto value : denotation)
            {
                if (stop())
                    return;
                registers->concept_values = context.registers.get_data().concept_values;
                registers->role_values = context.registers.get_data().role_values;
                apply_binding(rule, value, *registers);
                const auto target_registers = get_or_create(*m_task_context->dl_denotation_repository, *registers).first;
                if (!rule.get_effects().empty())
                {
                    m_environment.get_dl_target_caches().clear(false);
                    auto transition = m_environment.make_dl_transition_context(context.state,
                                                                               context.state,
                                                                               context.arguments,
                                                                               context.registers,
                                                                               target_registers);
                    if (!is_compatible_with(rule, transition))
                        continue;
                }
                auto target = context;
                target.registers = target_registers;
                target.memory_state = rule.get_target();
                result.push_back(applied(std::move(target), rule_variant));
            }
            if constexpr (ChooseRuleView<R>)
                if (!stop() && result.size() == initial_size)
                {
                    auto failure = make_step(detail::ProgramOutcome::FAILURE, context);
                    failure.rule = rule_variant;
                    result.push_back(std::move(failure));
                }
        }
        else if constexpr (std::same_as<R, RuleView<DoTag>>)
        {
            if (!rule_is_applicable(rule, context))
                return;

            const auto& denotations = evaluate_do_arguments(rule, context);
            visit_successors(
                [&](const LabeledNode& successor)
                {
                    m_environment.get_dl_target_caches().clear(false);
                    if (!do_successor_matches(rule, context, denotations, successor.label, successor.node.get_state()))
                        return;

                    auto target = context;
                    apply_do_successor(rule, successor, target);
                    result.push_back(planning_step(std::move(target), successor, rule_variant));
                },
                stop, rule, context, denotations);
        }
        else if constexpr (std::same_as<R, RuleView<ActionTag>>)
        {
            if (!rule_is_applicable(rule, context))
                return;
            auto state_context = m_environment.make_dl_context(context.state, context.arguments, context.registers);
            const auto query = evaluate(rule.get_query_feature(), state_context);
            m_action_rule_evaluator.action(rule, context.state, query.arity());
            visit_successors(
                [&](const LabeledNode& successor, bool binding_checked = false)
                {
                    m_environment.get_dl_target_caches().clear(false);
                    if (!action_successor_matches(rule, context, query, successor, binding_checked))
                        return;
                    auto target = context;
                    target.state = successor.node.get_state();
                    target.memory_state = rule.get_target();
                    result.push_back(planning_step(std::move(target), successor, rule_variant));
                },
                stop, rule, context, query);
        }
        else if constexpr (std::same_as<R, RuleView<CallTag>>)
        {
            auto target = context;
            const auto status = execute_call(rule, target);
            if (status == RuleExecutionStatus::NOT_APPLICABLE)
                return;
            if (status == RuleExecutionStatus::MALFORMED_CALL)
                result.push_back(make_step(detail::ProgramOutcome::MALFORMED_CALL, std::move(target)));
            else
                result.push_back(applied(std::move(target), rule_variant));
        }
        else if constexpr (std::same_as<R, RuleView<SketchTag>>)
        {
            if (!rule_is_applicable(rule, context))
                return;
            if (rule.get_effects().empty())
            {
                auto target = context;
                if (execute_sketch(rule, target, {}) == RuleExecutionStatus::APPLIED)
                    result.push_back(applied(std::move(target), rule_variant));
                return;
            }

            visit_successors(
                [&](const LabeledNode& successor)
                {
                    auto target = context;
                    if (execute_sketch(rule, target, { successor }) == RuleExecutionStatus::APPLIED)
                        result.push_back(planning_step(std::move(target), successor, rule_variant));
                },
                stop, rule, context);
        }
        else
        {
            static_assert(ygg::dependent_false<R>::value, "unhandled rule kind in SuccessorExpander::append_steps");
        }
    }

    Step make_step(detail::ProgramOutcome status, Frame context)
    {
        return Step(status, intern(context), m_task_context);
    }

    Step fallback(Frame context)
    {
        if (restore_caller(context))
            return make_step(detail::ProgramOutcome::RESTORED_CALLER, std::move(context));
        return make_step(detail::ProgramOutcome::NO_APPLICABLE_ACTION, std::move(context));
    }

    Step applied(Frame context, RuleVariantView rule)
    {
        auto step = make_step(detail::ProgramOutcome::APPLIED, std::move(context));
        step.rule = rule;
        return step;
    }

    Step planning_step(Frame context, const LabeledNode& successor, RuleVariantView rule)
    {
        auto step = applied(std::move(context), rule);
        step.plan_suffix.push_back(successor.pack());
        step.state_transition = runir::datasets::StateGraphEdgeLabel { successor.label, ygg::float_t(1) };
        return step;
    }

    bool selects(RuleVariantView rule, const Frame& context, const LabeledNode& candidate)
    {
        return ygg::visit(
            [&](auto concrete) -> bool
            {
                using R = std::decay_t<decltype(concrete)>;
                if constexpr (std::same_as<R, RuleView<SketchTag>>)
                    return sketch_rule_matches_state(concrete, context, candidate.node.get_state());
                else if constexpr (std::same_as<R, RuleView<DoTag>>)
                    return do_rule_matches(concrete, context, candidate.label, candidate.node.get_state());
                else if constexpr (std::same_as<R, RuleView<ActionTag>>)
                {
                    if (!rule_is_applicable(concrete, context))
                        return false;
                    auto state_context = m_environment.make_dl_context(context.state, context.arguments, context.registers);
                    const auto query = evaluate(concrete.get_query_feature(), state_context);
                    m_action_rule_evaluator.action(concrete, context.state, query.arity());
                    return action_successor_matches(concrete, context, query, candidate);
                }
                else if constexpr (BindingRuleView<R> || std::same_as<R, RuleView<CallTag>>)
                    return false;
                else
                    static_assert(ygg::dependent_false<R>::value, "unhandled rule kind in SuccessorExpander::selects");
            },
            rule.get_variant());
    }

    runir::kr::TaskContextPtr<Kind> m_task_context;
    ProgramView m_program;
    tyr::planning::ConjunctiveGoalStrategy<Kind> m_goal_strategy;
    tyr::planning::StateView<Kind> m_initial_state;
    bool m_static_goal_satisfied;
    EvaluationEnvironment<Kind> m_environment;
    detail::ActionRuleEvaluator<Kind> m_action_rule_evaluator;
    std::vector<ygg::uint_t> m_action_tuple;
    tyr::planning::PackedLabeledNodeList<Kind> m_all_successors;
    bool m_all_successors_ready = false;
};

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class SuccessorExpander<tyr::GroundTag>;
extern template class SuccessorExpander<tyr::LiftedTag>;

#endif

}  // namespace runir::kr::ps::ext

#endif
