#ifndef RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_

// Single source of module-program execution steps. Universal search explores all returned
// steps; greedy execution takes the first.

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/ext/detail/execution.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_repository.hpp"
#include "runir/kr/ps/ext/module_program_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <optional>
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

template<tyr::planning::TaskKind Kind>
class SuccessorExpander
{
public:
    using LabeledNode = tyr::planning::LabeledNode<Kind>;
    using Step = detail::ModuleProgramStep<Kind>;

    SuccessorExpander(runir::kr::TaskContextPtr<Kind> task_context, ModuleProgramView program) :
        SuccessorExpander(ExecutionRepository<Kind>::create(std::move(task_context), program))
    {
    }

    explicit SuccessorExpander(ExecutionRepositoryPtr<Kind> repository) :
        m_repository(std::move(repository)),
        m_goal_strategy(*m_repository->get_task_context().search_context->task),
        m_initial_state(m_repository->get_task_context().search_context->successor_generator->get_initial_node().get_state()),
        m_static_goal_satisfied(m_goal_strategy.is_static_goal_satisfied(*m_repository->get_task_context().search_context->task)),
        m_environment(m_repository)
    {
    }

    const auto& get_repository() const noexcept { return m_repository; }

    ExecutionStateView<Kind> initial_state()
    {
        const auto program = m_repository->get_program();
        auto context = EvaluationContext<Kind>(m_repository, m_initial_state, program.get_entry_module());
        return context.intern(ExecutionPhase::EXTERNAL);
    }

    bool is_goal(const tyr::planning::StateView<Kind>& state)
    {
        return m_static_goal_satisfied && m_goal_strategy.is_dynamic_goal_satisfied(m_initial_state, state);
    }

    bool is_goal(ExecutionStateView<Kind> state) { return is_goal(state.get_state()); }

    std::vector<Step> load_steps(ExecutionStateView<Kind> state)
    {
        return load_steps_until(state, [] { return false; });
    }

    std::vector<Step> load_steps_until(ExecutionStateView<Kind> state, auto&& stop)
    {
        auto context = EvaluationContext<Kind>(state);
        return collect_steps<LoadTag<runir::kr::dl::ConceptTag>, LoadTag<runir::kr::dl::RoleTag>>(context, {}, stop);
    }

    std::vector<LabeledNode> labeled_successors(ExecutionStateView<Kind> state)
    {
        auto& successor_generator = *m_repository->get_task_context().search_context->successor_generator;
        const auto node = successor_generator.get_node(state.get_state().get_index());
        return successor_generator.get_labeled_successor_nodes(node);
    }

    std::vector<Step> control_steps(ExecutionStateView<Kind> state, const std::vector<LabeledNode>& successors)
    {
        return control_steps_until(state, successors, [] { return false; });
    }

    std::vector<Step> control_steps_until(ExecutionStateView<Kind> state, const std::vector<LabeledNode>& successors, auto&& stop)
    {
        auto context = EvaluationContext<Kind>(state);
        auto immediate = collect_steps<DoTag, CallTag>(context, successors, stop);
        if (stop() || !immediate.empty())
            return immediate;

        auto sketch = collect_steps<SketchTag>(context, successors, stop);
        if (stop() || !sketch.empty())
            return sketch;

        auto target = context;
        if (target.get_call_stack().restore_caller())
            return std::vector<Step> { make_step(detail::ModuleProgramOutcome::RESTORED_CALLER, std::move(target), ExecutionPhase::EXTERNAL) };

        return std::vector<Step> { make_step(detail::ModuleProgramOutcome::NO_APPLICABLE_ACTION, std::move(context), ExecutionPhase::EXTERNAL) };
    }

    std::vector<Step> control_steps(ExecutionStateView<Kind> state) { return control_steps(state, labeled_successors(state)); }

    std::optional<RuleVariantView>
    matching_rule(ExecutionStateView<Kind> state, tyr::formalism::planning::GroundActionView action, tyr::planning::StateView<Kind> target_state)
    {
        auto context = EvaluationContext<Kind>(state);
        return matching_rule_for_candidate(context, LabeledNode { action, tyr::planning::Node<Kind>(std::move(target_state), ygg::float_t(0)) });
    }

    std::optional<Step> apply(ExecutionStateView<Kind> state,
                              RuleVariantView rule,
                              std::optional<tyr::formalism::planning::GroundActionView> action = std::nullopt,
                              std::optional<tyr::planning::StateView<Kind>> target_state = std::nullopt)
    {
        auto context = EvaluationContext<Kind>(state);
        std::optional<LabeledNode> candidate;
        if (action && target_state)
            candidate = LabeledNode { *action, tyr::planning::Node<Kind>(*target_state, ygg::float_t(0)) };
        return apply_rule(context, rule, std::move(candidate));
    }

private:
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
        ygg::visit([&](auto concrete) { append_steps(concrete, rule, context, successors, steps, stop); }, rule.get_variant());
        return steps.empty() ? std::nullopt : std::optional(std::move(steps.front()));
    }

    template<typename... Kinds>
    std::vector<Step> collect_steps(const EvaluationContext<Kind>& context, const std::vector<LabeledNode>& successors, auto&& stop)
    {
        auto result = std::vector<Step> {};
        for (const auto& transition : context.get_call_stack().module().get_memory_transitions())
        {
            for (auto rule_variant : transition)
            {
                if (stop())
                    return result;
                ygg::visit(
                    [&](auto rule)
                    {
                        using R = std::decay_t<decltype(rule)>;
                        if constexpr ((std::same_as<R, RuleView<Kinds>> || ...))
                            append_steps(rule, rule_variant, context, successors, result, stop);
                    },
                    rule_variant.get_variant());
            }
        }
        return result;
    }

    template<typename R>
    void append_steps(R rule,
                      RuleVariantView rule_variant,
                      const EvaluationContext<Kind>& context,
                      const std::vector<LabeledNode>& successors,
                      std::vector<Step>& result,
                      auto&& stop)
    {
        if constexpr (LoadRuleView<R>)
        {
            auto evaluation_context = context;
            if (!detail::load_rule_is_applicable(rule, evaluation_context, m_environment))
                return;

            const auto denotation = evaluate_feature_denotation(rule.get_feature(), evaluation_context, m_environment);
            if (denotation.begin() == denotation.end())
            {
                result.push_back(terminal(detail::ModuleProgramOutcome::FAILURE, context, rule_variant, ExecutionPhase::INTERNAL));
                return;
            }

            for (const auto value : denotation)
            {
                if (stop())
                    return;
                auto target = context;
                detail::apply_load_binding(rule, value, target);
                result.push_back(applied(std::move(target), rule_variant, ExecutionPhase::INTERNAL));
            }
        }
        else if constexpr (std::same_as<R, RuleView<DoTag>>)
        {
            auto evaluation_context = context;
            if (!detail::do_rule_is_applicable(rule, evaluation_context, m_environment))
                return;

            const auto& denotations = detail::evaluate_do_arguments(rule, evaluation_context, m_environment);
            auto matched = false;
            for (const auto& successor : successors)
            {
                if (stop())
                    return;
                if (!detail::do_successor_matches(rule, evaluation_context, m_environment, denotations, successor.label, successor.node.get_state()))
                    continue;

                matched = true;
                auto target = context;
                detail::apply_do_successor(rule, successor, target);
                result.push_back(planning_step(std::move(target), successor, rule_variant));
            }

            if (!matched)
            {
                auto target = context;
                const auto status = target.get_call_stack().restore_caller() ? detail::ModuleProgramOutcome::RESTORED_CALLER :
                                                                               detail::ModuleProgramOutcome::NO_APPLICABLE_ACTION;
                result.push_back(make_step(status, std::move(target), ExecutionPhase::EXTERNAL));
            }
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
            if (rule.get_effects().empty())
            {
                auto target = context;
                if (detail::execute_sketch(rule, target, m_environment, {}) == detail::RuleExecutionStatus::APPLIED)
                    result.push_back(applied(std::move(target), rule_variant, ExecutionPhase::EXTERNAL));
                return;
            }

            for (const auto& successor : successors)
            {
                if (stop())
                    return;
                auto target = context;
                if (detail::execute_sketch(rule, target, m_environment, { successor }) == detail::RuleExecutionStatus::APPLIED)
                    result.push_back(planning_step(std::move(target), successor, rule_variant));
            }
        }
        else
        {
            static_assert(ygg::dependent_false<R>::value, "unhandled rule kind in SuccessorExpander::append_steps");
        }
    }

    static Step make_step(detail::ModuleProgramOutcome status, EvaluationContext<Kind> context, ExecutionPhase phase)
    {
        return Step(status, context.intern(phase));
    }

    static Step applied(EvaluationContext<Kind> context, RuleVariantView rule, ExecutionPhase phase)
    {
        auto step = make_step(detail::ModuleProgramOutcome::APPLIED, std::move(context), phase);
        step.rule = rule;
        return step;
    }

    static Step terminal(detail::ModuleProgramOutcome status, EvaluationContext<Kind> context, RuleVariantView rule, ExecutionPhase phase)
    {
        auto step = make_step(status, std::move(context), phase);
        step.rule = rule;
        return step;
    }

    static Step planning_step(EvaluationContext<Kind> context, const LabeledNode& successor, RuleVariantView rule)
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
                else if constexpr (LoadRuleView<R> || std::same_as<R, RuleView<CallTag>>)
                    return false;
                else
                    static_assert(ygg::dependent_false<R>::value, "unhandled rule kind in SuccessorExpander::selects");
            },
            rule.get_variant());
    }

    ExecutionRepositoryPtr<Kind> m_repository;
    tyr::planning::ConjunctiveGoalStrategy<Kind> m_goal_strategy;
    tyr::planning::StateView<Kind> m_initial_state;
    bool m_static_goal_satisfied;
    EvaluationEnvironment<Kind> m_environment;
};

}  // namespace runir::kr::ps::ext

#endif
