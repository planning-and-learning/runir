#ifndef RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_

// Single source of module-program execution steps. Choose bindings are alternative
// continuations; ordinary successors are checked universally or selected greedily.

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/ext/detail/action_rule.hpp"
#include "runir/kr/ps/ext/detail/execution.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"
#include "runir/kr/ps/ext/expansion_policy.hpp"
#include "runir/kr/ps/ext/module_program_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
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
public:
    using LabeledNode = tyr::planning::LabeledNode<Kind>;
    using Step = detail::ModuleProgramStep<Kind>;

    SuccessorExpander(runir::kr::TaskContextPtr<Kind> task_context, ModuleProgramView program) :
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

    ExecutionStateView<Kind> initial_state()
    {
        auto context = EvaluationContext<Kind>(m_task_context->execution_repository.get(),
                                               &m_task_context->execution_builder,
                                               m_program,
                                               m_initial_state,
                                               m_program.get_entry_module());
        return context.intern(ExecutionPhase::EXTERNAL);
    }

    bool is_goal(const tyr::planning::StateView<Kind>& state)
    {
        return m_static_goal_satisfied && m_goal_strategy.is_dynamic_goal_satisfied(m_initial_state, state);
    }

    bool is_goal(ExecutionStateView<Kind> state) { return is_goal(state.get_state()); }

    std::vector<Step> load_steps(ExecutionStateView<Kind> state)
    {
        auto result = std::vector<Step> {};
        load_steps(std::move(state), result);
        return result;
    }

    void load_steps(ExecutionStateView<Kind> state, std::vector<Step>& out_steps)
    {
        load_steps_until(std::move(state), [] { return false; }, out_steps);
    }

    std::vector<Step> load_steps_until(ExecutionStateView<Kind> state, auto&& stop)
    {
        auto result = std::vector<Step> {};
        load_steps_until(std::move(state), stop, result);
        return result;
    }

    void load_steps_until(ExecutionStateView<Kind> state, auto&& stop, std::vector<Step>& out_steps)
    {
        auto context = EvaluationContext<Kind>(m_task_context->execution_repository.get(), &m_task_context->execution_builder, m_program, state);
        collect_steps<EagerExpansionPolicy, LoadTag<runir::kr::dl::ConceptTag>, LoadTag<runir::kr::dl::RoleTag>>(context, generated_successors<EagerExpansionPolicy>(), stop, out_steps);
    }

    std::vector<Step> choose_steps(ExecutionStateView<Kind> state)
    {
        auto result = std::vector<Step> {};
        choose_steps(std::move(state), result);
        return result;
    }

    void choose_steps(ExecutionStateView<Kind> state, std::vector<Step>& out_steps)
    {
        choose_steps_until(std::move(state), [] { return false; }, out_steps);
    }

    std::vector<Step> choose_steps_until(ExecutionStateView<Kind> state, auto&& stop)
    {
        auto result = std::vector<Step> {};
        choose_steps_until(std::move(state), stop, result);
        return result;
    }

    void choose_steps_until(ExecutionStateView<Kind> state, auto&& stop, std::vector<Step>& out_steps)
    {
        auto context = EvaluationContext<Kind>(m_task_context->execution_repository.get(), &m_task_context->execution_builder, m_program, state);
        collect_steps<EagerExpansionPolicy, ChooseTag<runir::kr::dl::ConceptTag>, ChooseTag<runir::kr::dl::RoleTag>>(context, generated_successors<EagerExpansionPolicy>(), stop, out_steps);
    }

    std::vector<LabeledNode> labeled_successors(ExecutionStateView<Kind> state)
    {
        auto result = std::vector<LabeledNode> {};
        labeled_successors(std::move(state), result);
        return result;
    }

    void labeled_successors(ExecutionStateView<Kind> state, std::vector<LabeledNode>& out_successors)
    {
        auto& search_context = *m_task_context->search_context;
        auto& successor_generator = *search_context.successor_generator;
        const auto node = successor_generator.get_node(*search_context.state_repository, state.get_state().get_index());
        successor_generator.get_labeled_successor_nodes(node, *search_context.state_repository, *search_context.axiom_evaluator, out_successors);
    }

    std::vector<Step> control_steps(ExecutionStateView<Kind> state, const std::vector<LabeledNode>& successors)
    {
        auto result = std::vector<Step> {};
        control_steps(std::move(state), successors, result);
        return result;
    }

    void control_steps(ExecutionStateView<Kind> state, const std::vector<LabeledNode>& successors, std::vector<Step>& out_steps)
    {
        control_steps_until(std::move(state), successors, [] { return false; }, out_steps);
    }

    std::vector<Step> control_steps_until(ExecutionStateView<Kind> state, const std::vector<LabeledNode>& successors, auto&& stop)
    {
        auto result = std::vector<Step> {};
        control_steps_until(std::move(state), successors, stop, result);
        return result;
    }

    void control_steps_until(ExecutionStateView<Kind> state, const std::vector<LabeledNode>& successors, auto&& stop, std::vector<Step>& out_steps)
    {
        auto context = EvaluationContext<Kind>(m_task_context->execution_repository.get(), &m_task_context->execution_builder, m_program, state);
        collect_steps<EagerExpansionPolicy, DoTag, ActionTag, CallTag, SketchTag>(context, supplied_successors(successors), stop, out_steps);
        if (!stop() && out_steps.empty())
            out_steps.push_back(fallback(std::move(context)));
    }

    std::vector<Step> control_steps(ExecutionStateView<Kind> state)
    {
        auto result = std::vector<Step> {};
        control_steps(std::move(state), result);
        return result;
    }

    void control_steps(ExecutionStateView<Kind> state, std::vector<Step>& out_steps)
    {
        auto context = EvaluationContext<Kind>(m_task_context->execution_repository.get(), &m_task_context->execution_builder, m_program, state);
        collect_steps<EagerExpansionPolicy, DoTag, ActionTag, CallTag, SketchTag>(context, generated_successors<EagerExpansionPolicy>(), [] { return false; }, out_steps);
        if (out_steps.empty())
            out_steps.push_back(fallback(std::move(context)));
    }

    std::vector<Step> steps(ExecutionStateView<Kind> state)
    {
        auto result = std::vector<Step> {};
        steps(std::move(state), result);
        return result;
    }

    void steps(ExecutionStateView<Kind> state, std::vector<Step>& out_steps)
    {
        steps_until(std::move(state), [] { return false; }, out_steps);
    }

    std::vector<Step> steps_until(ExecutionStateView<Kind> state, auto&& stop)
    {
        auto result = std::vector<Step> {};
        steps_until(std::move(state), stop, result);
        return result;
    }

    template<ExpansionPolicy Policy = EagerExpansionPolicy>
    void steps_until(ExecutionStateView<Kind> state, auto&& stop, std::vector<Step>& out_steps)
    {
        auto context = EvaluationContext<Kind>(m_task_context->execution_repository.get(), &m_task_context->execution_builder, m_program, state);
        collect_steps<Policy, LoadTag<runir::kr::dl::ConceptTag>, LoadTag<runir::kr::dl::RoleTag>, ChooseTag<runir::kr::dl::ConceptTag>,
                      ChooseTag<runir::kr::dl::RoleTag>, DoTag, ActionTag, CallTag, SketchTag>(context, generated_successors<Policy>(), stop, out_steps);
        if (!stop() && out_steps.empty())
            out_steps.push_back(fallback(std::move(context)));
    }

    std::vector<Step> steps(ExecutionStateView<Kind> state, const std::vector<LabeledNode>& successors)
    {
        auto result = std::vector<Step> {};
        steps(std::move(state), successors, result);
        return result;
    }

    void steps(ExecutionStateView<Kind> state, const std::vector<LabeledNode>& successors, std::vector<Step>& out_steps)
    {
        steps_until(std::move(state), successors, [] { return false; }, out_steps);
    }

    std::vector<Step> steps_until(ExecutionStateView<Kind> state, const std::vector<LabeledNode>& successors, auto&& stop)
    {
        auto result = std::vector<Step> {};
        steps_until(std::move(state), successors, stop, result);
        return result;
    }

    template<ExpansionPolicy Policy = EagerExpansionPolicy>
    void steps_until(ExecutionStateView<Kind> state, const std::vector<LabeledNode>& successors, auto&& stop, std::vector<Step>& out_steps)
    {
        auto context = EvaluationContext<Kind>(m_task_context->execution_repository.get(), &m_task_context->execution_builder, m_program, state);
        collect_steps<Policy, LoadTag<runir::kr::dl::ConceptTag>, LoadTag<runir::kr::dl::RoleTag>, ChooseTag<runir::kr::dl::ConceptTag>,
                      ChooseTag<runir::kr::dl::RoleTag>, DoTag, ActionTag, CallTag, SketchTag>(context, supplied_successors(successors), stop, out_steps);
        if (!stop() && out_steps.empty())
            out_steps.push_back(fallback(std::move(context)));
    }

    std::optional<RuleVariantView>
    matching_rule(ExecutionStateView<Kind> state, tyr::formalism::planning::ActionBindingView action, tyr::planning::StateView<Kind> target_state)
    {
        auto context = EvaluationContext<Kind>(m_task_context->execution_repository.get(), &m_task_context->execution_builder, m_program, state);
        return matching_rule_for_candidate(context, LabeledNode { action, tyr::planning::Node<Kind>(std::move(target_state), ygg::float_t(0)) });
    }

    std::optional<Step> apply(ExecutionStateView<Kind> state,
                              RuleVariantView rule,
                              std::optional<tyr::formalism::planning::ActionBindingView> action = std::nullopt,
                              std::optional<tyr::planning::StateView<Kind>> target_state = std::nullopt)
    {
        auto context = EvaluationContext<Kind>(m_task_context->execution_repository.get(), &m_task_context->execution_builder, m_program, state);
        std::optional<LabeledNode> candidate;
        if (action && target_state)
            candidate = LabeledNode { *action, tyr::planning::Node<Kind>(*target_state, ygg::float_t(0)) };
        return apply_rule(context, rule, std::move(candidate));
    }

private:
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

    template<ExpansionPolicy Policy>
    auto generated_successors()
    {
        m_all_successors_ready = false;
        m_all_action_bindings_ready = false;
        m_all_successors.clear();
        return [this](auto&& emit, auto&& stop, auto rule, auto&&... arguments)
        { this->template for_each_successor<Policy>(rule, std::forward<decltype(arguments)>(arguments)..., emit, stop); };
    }

    template<ExpansionPolicy Policy>
    void for_each_successor(RuleView<DoTag> rule, const EvaluationContext<Kind>& context, const auto& denotations, auto&& emit, auto&& stop)
    {
        if (stop() || std::ranges::any_of(denotations, [](const auto& denotation) { return denotation.get().count() == 0; }))
            return;
        if (m_all_successors_ready)
        {
            supplied_successors(m_all_successors)(emit, stop);
            return;
        }

        auto& search_context = *m_task_context->search_context;
        auto& generator = *search_context.successor_generator;
        if constexpr (std::same_as<Policy, LazyExpansionPolicy>)
        {
            if (m_all_action_bindings_ready)
            {
                const auto node = generator.get_node(*search_context.state_repository, context.get_state().get_index());
                for (const auto binding : m_all_action_bindings)
                {
                    if (stop())
                        return;
                    if (detail::action_matches_do_arguments(rule, binding, denotations))
                        emit(LabeledNode { binding, generator.get_successor_node(node, binding, *search_context.state_repository, *search_context.axiom_evaluator) });
                }
                return;
            }
        }
        for (const auto action : search_context.task->get_task().get_domain().get_actions())
        {
            if (action.get_name().str() != rule.get_action_name())
                continue;
            const auto node = generator.get_node(*search_context.state_repository, context.get_state().get_index());
            generator.get_applicable_action_bindings(node, action, m_action_bindings);
            for (const auto binding : m_action_bindings)
            {
                if (stop())
                    return;
                if (!detail::action_matches_do_arguments(rule, binding, denotations))
                    continue;
                emit(LabeledNode { binding,
                                   generator.get_successor_node(node, binding, *search_context.state_repository, *search_context.axiom_evaluator) });
            }
            return;
        }
    }

    template<ExpansionPolicy Policy>
    void for_each_successor(RuleView<SketchTag>, const EvaluationContext<Kind>& context, auto&& emit, auto&& stop)
    {
        if (stop())
            return;
        auto& search_context = *m_task_context->search_context;
        auto& generator = *search_context.successor_generator;
        if constexpr (std::same_as<Policy, LazyExpansionPolicy>)
        {
            const auto node = generator.get_node(*search_context.state_repository, context.get_state().get_index());
            if (!m_all_action_bindings_ready)
            {
                generator.get_applicable_action_bindings(node, m_all_action_bindings);
                m_all_action_bindings_ready = true;
            }
            for (const auto binding : m_all_action_bindings)
            {
                if (stop())
                    return;
                emit(LabeledNode { binding, generator.get_successor_node(node, binding, *search_context.state_repository, *search_context.axiom_evaluator) });
            }
        }
        else
        {
            if (!m_all_successors_ready)
            {
                const auto node = generator.get_node(*search_context.state_repository, context.get_state().get_index());
                generator.get_labeled_successor_nodes(node, *search_context.state_repository, *search_context.axiom_evaluator, m_all_successors);
                m_all_successors_ready = true;
            }
            supplied_successors(m_all_successors)(emit, stop);
        }
    }

    template<ExpansionPolicy Policy>
    void for_each_successor(RuleView<ActionTag> rule,
                            const EvaluationContext<Kind>& context,
                            const ygg::database::Relation<>& query,
                            auto&& emit,
                            auto&& stop)
    {
        if (stop() || query.empty())
            return;
        auto& search_context = *m_task_context->search_context;
        auto& generator = *search_context.successor_generator;
        const auto node = generator.get_node(*search_context.state_repository, context.get_state().get_index());
        for (size_t i = 0; i < query.size(); ++i)
        {
            if (stop())
                return;
            const auto binding = m_action_rule_evaluator.applicable_binding(rule, node, query[i]);
            emit(LabeledNode { binding, generator.get_successor_node(node, binding, *search_context.state_repository, *search_context.axiom_evaluator) }, true);
        }
    }

    bool action_successor_matches(RuleView<ActionTag> rule,
                                  EvaluationContext<Kind>& context,
                                  const ygg::database::Relation<>& query,
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
            const auto node = search_context.successor_generator->get_node(*search_context.state_repository, context.get_state().get_index());
            m_action_rule_evaluator.applicable_binding(rule, node, m_action_tuple);
        }
        auto transition = m_environment.make_dl_transition_context(context, candidate.node.get_state());
        for (const auto effect : rule.get_effects())
            if (!is_compatible_with(effect, transition))
                detail::action_rule_contract_error(rule, context.get_state(), m_action_tuple, "offered transition violates declared effects");
        return true;
    }

    std::optional<RuleVariantView> matching_rule_for_candidate(EvaluationContext<Kind>& context, const LabeledNode& candidate)
    {
        for (const auto& transition : context.get_call_stack().module().get_memory_transitions())
            for (auto rule : transition)
                if (selects(rule, context, candidate))
                    return rule;
        return std::nullopt;
    }

    std::optional<Step> apply_rule(const EvaluationContext<Kind>& context, RuleVariantView rule, std::optional<LabeledNode> candidate = std::nullopt)
    {
        const auto successors = candidate ? std::vector<LabeledNode> { *candidate } : std::vector<LabeledNode> {};
        auto steps = std::vector<Step> {};
        const auto stop = [] { return false; };
        ygg::visit([&](auto concrete) { append_steps<EagerExpansionPolicy>(concrete, rule, context, supplied_successors(successors), steps, stop); }, rule.get_variant());
        return steps.empty() ? std::nullopt : std::optional(std::move(steps.front()));
    }

    template<ExpansionPolicy Policy, typename... Kinds>
    void collect_steps(const EvaluationContext<Kind>& context, auto&& visit_successors, auto&& stop, std::vector<Step>& out_steps)
    {
        out_steps.clear();
        for (const auto& transition : context.get_call_stack().module().get_memory_transitions())
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
                      const EvaluationContext<Kind>& context,
                      auto&& visit_successors,
                      std::vector<Step>& result,
                      auto&& stop)
    {
        if constexpr (BindingRuleView<R>)
        {
            auto evaluation_context = context;
            if (!detail::binding_rule_is_applicable(rule, evaluation_context, m_environment))
                return;

            const auto initial_size = result.size();
            const auto denotation = evaluate_feature_denotation(rule.get_feature(), evaluation_context, m_environment);
            for (const auto value : denotation)
            {
                if (stop())
                    return;
                auto target = context;
                detail::apply_binding(rule, value, target);
                if (!rule.get_effects().empty())
                {
                    auto transition = m_environment.make_dl_binding_context(context, target);
                    if (!is_compatible_with(rule, transition))
                        continue;
                }
                result.push_back(applied(std::move(target), rule_variant, ExecutionPhase::INTERNAL));
            }
            if constexpr (ChooseRuleView<R>)
                if (!stop() && result.size() == initial_size)
                {
                    auto failure = make_step(detail::ModuleProgramOutcome::FAILURE, context, ExecutionPhase::INTERNAL);
                    failure.rule = rule_variant;
                    result.push_back(std::move(failure));
                }
        }
        else if constexpr (std::same_as<R, RuleView<DoTag>>)
        {
            auto evaluation_context = context;
            if (!detail::do_rule_is_applicable(rule, evaluation_context, m_environment))
                return;

            const auto& denotations = detail::evaluate_do_arguments(rule, evaluation_context, m_environment);
            visit_successors(
                [&](const LabeledNode& successor)
                {
                    if (!detail::do_successor_matches(rule, evaluation_context, m_environment, denotations, successor.label, successor.node.get_state()))
                        return;

                    auto target = context;
                    detail::apply_do_successor(rule, successor, target);
                    result.push_back(planning_step(std::move(target), successor, rule_variant));
                },
                stop, rule, evaluation_context, denotations);
        }
        else if constexpr (std::same_as<R, RuleView<ActionTag>>)
        {
            auto evaluation_context = context;
            if (!detail::has_current_source(rule, evaluation_context) || !conditions_are_compatible(rule, evaluation_context, m_environment))
                return;
            const auto query = evaluate_feature_denotation(rule.get_query_feature(), evaluation_context, m_environment);
            m_action_rule_evaluator.action(rule, context.get_state(), query->arity());
            visit_successors(
                [&](const LabeledNode& successor, bool binding_checked = false)
                {
                    if (!action_successor_matches(rule, evaluation_context, *query, successor, binding_checked))
                        return;
                    auto target = context;
                    target.get_state() = successor.node.get_state();
                    target.get_call_stack().set_memory_state(rule.get_target());
                    result.push_back(planning_step(std::move(target), successor, rule_variant));
                },
                stop, rule, evaluation_context, *query);
        }
        else if constexpr (std::same_as<R, RuleView<CallTag>>)
        {
            auto target = context;
            const auto status = detail::execute_call(rule, target, m_environment);
            if (status == detail::RuleExecutionStatus::NOT_APPLICABLE)
                return;
            if (status == detail::RuleExecutionStatus::MALFORMED_CALL)
                result.push_back(make_step(detail::ModuleProgramOutcome::MALFORMED_CALL, std::move(target), ExecutionPhase::EXTERNAL));
            else
                result.push_back(applied(std::move(target), rule_variant, ExecutionPhase::EXTERNAL));
        }
        else if constexpr (std::same_as<R, RuleView<SketchTag>>)
        {
            auto evaluation_context = context;
            if (!detail::has_current_source(rule, evaluation_context) || !conditions_are_compatible(rule, evaluation_context, m_environment))
                return;
            if (rule.get_effects().empty())
            {
                auto target = context;
                if (detail::execute_sketch(rule, target, m_environment, {}) == detail::RuleExecutionStatus::APPLIED)
                    result.push_back(applied(std::move(target), rule_variant, ExecutionPhase::EXTERNAL));
                return;
            }

            visit_successors(
                [&](const LabeledNode& successor)
                {
                    auto target = context;
                    if (detail::execute_sketch(rule, target, m_environment, { successor }) == detail::RuleExecutionStatus::APPLIED)
                        result.push_back(planning_step(std::move(target), successor, rule_variant));
                },
                stop, rule, evaluation_context);
        }
        else
        {
            static_assert(ygg::dependent_false<R>::value, "unhandled rule kind in SuccessorExpander::append_steps");
        }
    }

    Step make_step(detail::ModuleProgramOutcome status, EvaluationContext<Kind> context, ExecutionPhase phase)
    {
        return Step(status, context.intern(phase), m_task_context);
    }

    Step fallback(EvaluationContext<Kind> context)
    {
        auto target = context;
        if (target.get_call_stack().restore_caller())
            return make_step(detail::ModuleProgramOutcome::RESTORED_CALLER, std::move(target), ExecutionPhase::EXTERNAL);
        return make_step(detail::ModuleProgramOutcome::NO_APPLICABLE_ACTION, std::move(context), ExecutionPhase::EXTERNAL);
    }

    Step applied(EvaluationContext<Kind> context, RuleVariantView rule, ExecutionPhase phase)
    {
        auto step = make_step(detail::ModuleProgramOutcome::APPLIED, std::move(context), phase);
        step.rule = rule;
        return step;
    }

    Step planning_step(EvaluationContext<Kind> context, const LabeledNode& successor, RuleVariantView rule)
    {
        auto step = applied(std::move(context), rule, ExecutionPhase::EXTERNAL);
        step.plan_suffix.push_back(successor);
        step.state_transition = runir::datasets::StateGraphEdgeLabel { successor.label, ygg::float_t(1) };
        return step;
    }

    bool selects(RuleVariantView rule, EvaluationContext<Kind>& context, const LabeledNode& candidate)
    {
        return ygg::visit(
            [&](auto concrete) -> bool
            {
                using R = std::decay_t<decltype(concrete)>;
                if constexpr (std::same_as<R, RuleView<SketchTag>>)
                    return detail::sketch_rule_matches_state(concrete, context, m_environment, candidate.node.get_state());
                else if constexpr (std::same_as<R, RuleView<DoTag>>)
                    return detail::do_rule_matches(concrete, context, m_environment, candidate.label, candidate.node.get_state());
                else if constexpr (std::same_as<R, RuleView<ActionTag>>)
                {
                    if (!detail::has_current_source(concrete, context) || !conditions_are_compatible(concrete, context, m_environment))
                        return false;
                    const auto query = evaluate_feature_denotation(concrete.get_query_feature(), context, m_environment);
                    m_action_rule_evaluator.action(concrete, context.get_state(), query->arity());
                    return action_successor_matches(concrete, context, *query, candidate);
                }
                else if constexpr (BindingRuleView<R> || std::same_as<R, RuleView<CallTag>>)
                    return false;
                else
                    static_assert(ygg::dependent_false<R>::value, "unhandled rule kind in SuccessorExpander::selects");
            },
            rule.get_variant());
    }

    runir::kr::TaskContextPtr<Kind> m_task_context;
    ModuleProgramView m_program;
    tyr::planning::ConjunctiveGoalStrategy<Kind> m_goal_strategy;
    tyr::planning::StateView<Kind> m_initial_state;
    bool m_static_goal_satisfied;
    EvaluationEnvironment<Kind> m_environment;
    detail::ActionRuleEvaluator<Kind> m_action_rule_evaluator;
    std::vector<ygg::uint_t> m_action_tuple;
    std::vector<tyr::formalism::planning::ActionBindingView> m_action_bindings;
    std::vector<tyr::formalism::planning::ActionBindingView> m_all_action_bindings;
    bool m_all_action_bindings_ready = false;
    std::vector<LabeledNode> m_all_successors;
    bool m_all_successors_ready = false;
};

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class SuccessorExpander<tyr::GroundTag>;
extern template class SuccessorExpander<tyr::LiftedTag>;

#endif

}  // namespace runir::kr::ps::ext

#endif
