#ifndef RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_

// Single source of module-program execution steps. The proof search explores all returned
// steps; greedy execution takes the first.

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/ext/detail/execution.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/memory_state.hpp"
#include "runir/kr/ps/ext/module_program_proof_graph.hpp"
#include "runir/kr/ps/ext/module_program_view.hpp"
#include "runir/kr/ps/ext/module_view.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/task_context.hpp"

#include <limits>
#include <optional>
#include <type_traits>
#include <tyr/planning/algorithms/strategies/goal.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/dependent_false.hpp>

namespace runir::kr::ps::ext
{

template<tyr::planning::TaskKind Kind>
class SuccessorExpander
{
public:
    using VertexLabel = ModuleProgramProofVertexLabel<Kind>;
    using EdgeLabel = ModuleProgramProofEdgeLabel;
    using Successor = std::pair<EdgeLabel, VertexLabel>;
    using LabeledNode = tyr::planning::LabeledNode<Kind>;
    using Step = detail::ModuleProgramStep<Kind>;

    SuccessorExpander(runir::kr::TaskContext<Kind>& task_context, const tyr::planning::Node<Kind>& initial_node, ModuleProgramView program) :
        m_task_context(&task_context),
        m_goal_strategy(*task_context.search_context->task),
        m_initial_state(initial_node.get_state()),
        m_static_goal_satisfied(m_goal_strategy.is_static_goal_satisfied(*task_context.search_context->task)),
        m_environment(task_context, program)
    {
    }

    SuccessorExpander(runir::kr::TaskContext<Kind>& task_context, tyr::planning::StateView<Kind> initial_state, ModuleProgramView program) :
        SuccessorExpander(task_context, tyr::planning::Node<Kind>(std::move(initial_state), ygg::float_t(0)), program)
    {
    }

    EvaluationContext<Kind> context_at(ModuleView module, MemoryStateView memory_state, Registers registers, tyr::planning::StateView<Kind> source_state)
    {
        return EvaluationContext<Kind>(std::move(source_state), m_environment.get_program(), module, memory_state, std::move(registers));
    }

    EvaluationContext<Kind> context_at_vertex(const VertexLabel& label)
    {
        const auto memory_state = std::visit([](const auto& value) { return value.value; }, label.extended_state.memory_state);
        auto registers = Registers { label.extended_state.concept_registers, label.extended_state.role_registers };
        return EvaluationContext<Kind>(label.extended_state.annotated_state.state,
                                       m_environment.get_program(),
                                       label.module_,
                                       memory_state,
                                       std::move(registers),
                                       label.arguments,
                                       label.frames);
    }

    EvaluationContext<Kind> initial_context()
    {
        const auto program = m_environment.get_program();
        return EvaluationContext<Kind>(m_initial_state, program, program.get_entry_module());
    }

    bool is_goal(const tyr::planning::StateView<Kind>& state)
    {
        return m_static_goal_satisfied && m_goal_strategy.is_dynamic_goal_satisfied(m_initial_state, state);
    }

    // The one node constructor, shared with `ModuleProgramProofBuilder`. `is_alive`/`is_unsolvable`
    // are supplied by the caller: the search assigns them from its analysis; off-search callers
    // (the frontier) pass a live, non-unsolvable node since only the search can refine them.
    VertexLabel make_vertex_label(const EvaluationContext<Kind>& context, MemoryStateVariant memory_state, bool is_initial, bool is_alive, bool is_unsolvable)
    {
        const auto goal = is_goal(context.get_state());
        auto annotated = runir::datasets::AnnotatedStateGraphVertexLabel<Kind> {
            context.get_state(), goal ? ygg::float_t(0) : std::numeric_limits<ygg::float_t>::infinity(), is_initial, goal, is_alive, is_unsolvable,
        };
        return VertexLabel { ExtendedState<Kind> { annotated,
                                                   std::move(memory_state),
                                                   context.get_call_stack().registers().template get<runir::kr::dl::ConceptTag>(),
                                                   context.get_call_stack().registers().template get<runir::kr::dl::RoleTag>() },
                             context.get_call_stack().module(),
                             context.get_call_stack().arguments(),
                             context.get_call_stack().frames() };
    }

    // The internal (Load) moves at the vertex: each holds the planning state fixed and changes the
    // memory state by binding a register. An empty-denotation Load yields a FAILURE step. This is
    // the universal/greedy load enumeration.
    std::vector<Step> load_steps(const EvaluationContext<Kind>& context)
    {
        return load_steps_until(context, [] { return false; });
    }

    std::vector<Step> load_steps_until(const EvaluationContext<Kind>& context, auto&& stop)
    {
        return collect_steps<LoadTag<runir::kr::dl::ConceptTag>, LoadTag<runir::kr::dl::RoleTag>>(context, {}, stop);
    }

    std::vector<LabeledNode> labeled_successors(const EvaluationContext<Kind>& context)
    {
        auto& successor_generator = *m_task_context->search_context->successor_generator;
        const auto node = successor_generator.get_node(context.get_state().get_index());
        return successor_generator.get_labeled_successor_nodes(node);
    }

    // Do and Call form the first control tier. Immediate Sketch transitions form the second.
    // If neither tier applies, execution restores the caller or terminates.
    std::vector<Step> control_steps(const EvaluationContext<Kind>& context, const std::vector<LabeledNode>& successors)
    {
        return control_steps_until(context, successors, [] { return false; });
    }

    std::vector<Step> control_steps_until(const EvaluationContext<Kind>& context, const std::vector<LabeledNode>& successors, auto&& stop)
    {
        auto immediate = collect_steps<DoTag, CallTag>(context, successors, stop);
        if (stop() || !immediate.empty())
            return immediate;

        auto sketch = collect_steps<SketchTag>(context, successors, stop);
        if (stop() || !sketch.empty())
            return sketch;

        auto target = context;
        if (target.get_call_stack().restore_caller())
            return std::vector<Step> { Step(detail::ModuleProgramOutcome::RESTORED_CALLER, std::move(target)) };

        return std::vector<Step> { Step(detail::ModuleProgramOutcome::NO_APPLICABLE_ACTION, context) };
    }

    std::vector<Step> control_steps(const EvaluationContext<Kind>& context) { return control_steps(context, labeled_successors(context)); }

    // The control rule (Sketch/Do) of the source vertex that selects the candidate planning
    // successor, or nullopt (the gap). Load/Call are internal memory steps, never a planning move.
    std::optional<RuleVariantView>
    matching_rule(EvaluationContext<Kind>& context, tyr::formalism::planning::GroundActionView action, tyr::planning::StateView<Kind> target_state)
    {
        return matching_rule_for_candidate(context, LabeledNode { action, tyr::planning::Node<Kind>(std::move(target_state), ygg::float_t(0)) });
    }

    std::optional<Successor> apply(const EvaluationContext<Kind>& context,
                                   RuleVariantView rule,
                                   std::optional<tyr::formalism::planning::GroundActionView> action = std::nullopt,
                                   std::optional<tyr::planning::StateView<Kind>> target_state = std::nullopt)
    {
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

    std::optional<Successor> apply_rule(const EvaluationContext<Kind>& context, RuleVariantView rule, std::optional<LabeledNode> candidate = std::nullopt)
    {
        const auto successors = candidate ? std::vector<LabeledNode> { *candidate } : std::vector<LabeledNode> {};
        auto steps = std::vector<Step> {};
        const auto stop = [] { return false; };
        ygg::visit([&](auto concrete) { append_steps(concrete, rule, context, successors, steps, stop); }, rule.get_variant());
        return steps.empty() ? std::nullopt : to_successor(steps.front());
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

            const auto denotation = detail::evaluate_load_expression(rule, evaluation_context, m_environment);
            if (denotation.begin() == denotation.end())
            {
                result.push_back(terminal(detail::ModuleProgramOutcome::FAILURE, context, rule_variant));
                return;
            }

            for (const auto value : denotation)
            {
                if (stop())
                    return;
                auto target = context;
                detail::apply_load_binding(rule, value, target);
                result.push_back(applied(std::move(target), rule_variant));
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
                result.emplace_back(target.get_call_stack().restore_caller() ? detail::ModuleProgramOutcome::RESTORED_CALLER :
                                                                               detail::ModuleProgramOutcome::NO_APPLICABLE_ACTION,
                                    std::move(target));
            }
        }
        else if constexpr (std::same_as<R, RuleView<CallTag>>)
        {
            auto target = context;
            const auto status = detail::execute_call(rule, target, m_environment);
            if (status == detail::RuleExecutionStatus::NOT_APPLICABLE)
                return;
            if (status == detail::RuleExecutionStatus::MALFORMED_CALL)
                result.emplace_back(detail::ModuleProgramOutcome::MALFORMED_CALL, context);
            else
                result.push_back(applied(std::move(target), rule_variant));
        }
        else if constexpr (std::same_as<R, RuleView<SketchTag>>)
        {
            if (rule.get_effects().empty())
            {
                auto target = context;
                if (detail::execute_sketch(rule, target, m_environment, {}) == detail::RuleExecutionStatus::APPLIED)
                    result.push_back(applied(std::move(target), rule_variant));
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

    static Step applied(EvaluationContext<Kind> context, RuleVariantView rule)
    {
        auto step = Step(detail::ModuleProgramOutcome::APPLIED, std::move(context));
        step.rule = rule;
        return step;
    }

    static Step terminal(detail::ModuleProgramOutcome status, EvaluationContext<Kind> context, RuleVariantView rule)
    {
        auto step = Step(status, std::move(context));
        step.rule = rule;
        return step;
    }

    static Step planning_step(EvaluationContext<Kind> context, const LabeledNode& successor, RuleVariantView rule)
    {
        auto step = applied(std::move(context), rule);
        step.plan_suffix.push_back(successor);
        step.state_transition = runir::datasets::StateGraphEdgeLabel { successor.label, ygg::float_t(1) };
        return step;
    }

    // Wrap an APPLIED step as a proof-graph (edge, node). Load advances internal memory; the control
    // rules advance external memory.
    std::optional<Successor> to_successor(const Step& step)
    {
        if (step.status != detail::ModuleProgramOutcome::APPLIED || !step.rule)
            return std::nullopt;

        auto memory_state = ygg::visit(
            [&](auto rule) -> MemoryStateVariant
            {
                using R = std::decay_t<decltype(rule)>;
                if constexpr (LoadRuleView<R>)
                    return InternalMemoryState(step.context.get_call_stack().memory_state());
                else if constexpr (std::same_as<R, RuleView<DoTag>> || std::same_as<R, RuleView<CallTag>> || std::same_as<R, RuleView<SketchTag>>)
                    return ExternalMemoryState(step.context.get_call_stack().memory_state());
                else
                    static_assert(ygg::dependent_false<R>::value, "unhandled rule kind in SuccessorExpander::to_successor");
            },
            step.rule->get_variant());

        auto vertex = make_vertex_label(step.context, std::move(memory_state), /*is_initial=*/false, /*is_alive=*/true, /*is_unsolvable=*/false);
        return Successor { EdgeLabel { step.state_transition, *step.rule }, std::move(vertex) };
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

    runir::kr::TaskContext<Kind>* m_task_context;
    tyr::planning::ConjunctiveGoalStrategy<Kind> m_goal_strategy;
    tyr::planning::StateView<Kind> m_initial_state;
    bool m_static_goal_satisfied;
    EvaluationEnvironment<Kind> m_environment;
};

}  // namespace runir::kr::ps::ext

#endif
