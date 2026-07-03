#ifndef RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_

// Single source of module-program successor expansion.
//
// `SuccessorExpander` is the one place a rule is applied to a source vertex (an `EvaluationContext`:
// state + memory + registers). Its private `make_step` runs the executor's `execute_*` for the rule
// and packages the result as a `ModuleProgramStep` (resulting context + planning transition + rule).
// Everything else is a view over `make_step`:
//   - `internal_steps` / `control_steps` enumerate the Load and Sketch/Do/Call moves; the prove
//     driver takes all of them and the greedy driver takes the first, so the two can never disagree.
//   - `apply` / `internal_successors` wrap a step as the proof graph's own
//     `ModuleProgramProofVertexLabel` / `ModuleProgramProofEdgeLabel`, the structures runir-mcp
//     renders, so the public frontier reuses the exact same construction.
// `ModuleProgramProofBuilder` also delegates its node construction (`make_vertex_label`) here.
//
// Rule kinds are dispatched with `if constexpr` over every variant alternative (with a
// `ygg::dependent_false` catch-all), so adding a rule kind is a compile error until handled here.

#include "runir/datasets/state_graph.hpp"
#include "runir/datasets/task_class.hpp"
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

#include <limits>
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

template<typename T>
struct IsLoadRuleView : std::false_type
{
};

template<runir::kr::dl::CategoryTag Category, typename C>
struct IsLoadRuleView<ygg::View<ygg::Index<Rule<LoadTag, Category>>, C>> : std::true_type
{
};

template<typename T>
inline constexpr bool is_load_rule_view_v = IsLoadRuleView<std::remove_cvref_t<T>>::value;

template<typename R, typename Kind>
struct MatchesRuleKind : std::bool_constant<std::same_as<R, RuleView<Kind>>>
{
};

template<typename R>
struct MatchesRuleKind<R, LoadTag> : std::bool_constant<is_load_rule_view_v<R>>
{
};

template<typename R, typename Kind>
inline constexpr bool matches_rule_kind_v = MatchesRuleKind<std::remove_cvref_t<R>, Kind>::value;

template<tyr::planning::TaskKind Kind>
class SuccessorExpander
{
private:
    static std::vector<ModuleView> collect_modules(ModuleProgramView program)
    {
        auto result = std::vector<ModuleView> {};
        for (auto module : program.get_modules())
            result.push_back(module);
        return result;
    }

public:
    using VertexLabel = ModuleProgramProofVertexLabel<Kind>;
    using EdgeLabel = ModuleProgramProofEdgeLabel;
    using Successor = std::pair<EdgeLabel, VertexLabel>;
    using LabeledNode = tyr::planning::LabeledNode<Kind>;
    using Step = detail::ModuleProgramStep<Kind>;

    SuccessorExpander(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      const tyr::planning::Node<Kind>& initial_node,
                      std::vector<ModuleView> modules = {}) :
        m_search_context(&search_context),
        m_goal_strategy(*search_context.task),
        m_initial_state(initial_node.get_state()),
        m_static_goal_satisfied(m_goal_strategy.is_static_goal_satisfied(*search_context.task)),
        m_environment(std::move(modules))
    {
    }

    SuccessorExpander(const runir::datasets::TaskSearchContext<Kind>& search_context,
                      tyr::planning::StateView<Kind> initial_state,
                      std::vector<ModuleView> modules = {}) :
        SuccessorExpander(search_context, tyr::planning::Node<Kind>(std::move(initial_state), ygg::float_t(0)), std::move(modules))
    {
    }

    EvaluationContext<Kind> context_at(ModuleView module,
                                       MemoryStateView memory_state,
                                       typename EvaluationContext<Kind>::ConceptRegisters concept_registers,
                                       typename EvaluationContext<Kind>::RoleRegisters role_registers,
                                       tyr::planning::StateView<Kind> source_state)
    {
        return m_environment.make_context(std::move(source_state), module, memory_state, std::move(concept_registers), std::move(role_registers));
    }

    EvaluationContext<Kind> initial_context(ModuleProgramView program)
    {
        if (m_environment.get_modules().empty())
            m_environment.set_modules(collect_modules(program));
        return m_environment.make_context(m_initial_state, program.get_entry_module());
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
        return VertexLabel { ExtendedState<Kind> { annotated, std::move(memory_state), context.concept_registers(), context.role_registers() },
                             context.get_module() };
    }

    // The internal (Load) moves at the vertex: each holds the planning state fixed and changes the
    // memory state by binding a register. An empty-denotation Load yields a FAILURE step. This is
    // the prove/greedy load enumeration.
    std::vector<Step> internal_steps(const EvaluationContext<Kind>& context)
    {
        ensure_modules(context);
        return collect_steps<LoadTag>(context, {});
    }
    std::vector<Step> load_successors(const EvaluationContext<Kind>& context) { return internal_steps(context); }

    // The control moves at the vertex, in the executor's priority order: immediate external rules
    // (Do/Call) first; if none fire, Sketch rules; if none fire, the IW transition search to a
    // state a Sketch rule matches; otherwise a single terminal step (restore caller / no applicable
    // action / out of time / out of states). The prove driver explores all returned steps; the
    // greedy driver takes the first.
    std::vector<Step> control_steps(const runir::datasets::TaskSearchContext<Kind>& search_context,
                                    const EvaluationContext<Kind>& context,
                                    const detail::ModuleExecutionOptions<Kind>& options)
    {
        ensure_modules(context);
        const auto node = search_context.successor_generator->get_node(context.get_state().get_index());
        const auto successors = search_context.successor_generator->get_labeled_successor_nodes(node);

        if (auto immediate = collect_steps<DoTag, CallTag>(context, successors); !immediate.empty())
            return immediate;

        if (auto sketch = collect_steps<SketchTag>(context, successors); !sketch.empty())
            return sketch;

        auto transition_context = context;
        const auto search_result = detail::find_module_program_transition_node(search_context, transition_context, options);
        if (search_result.status == tyr::planning::SearchStatus::SOLVED && search_result.goal_node)
        {
            auto match_context = context;
            if (const auto rule = detail::find_matching_sketch_rule(match_context, m_environment, search_result.goal_node->get_state()))
            {
                auto variant = detail::find_matching_sketch_rule_variant(match_context, m_environment, search_result.goal_node->get_state());
                auto target = context;
                target.set_state(search_result.goal_node->get_state());
                target.set_memory_state(rule->get_target());
                auto step = Step(detail::ModuleProgramOutcome::APPLIED, std::move(target));
                step.rule = variant;
                detail::append_plan_suffix(step.plan_suffix, search_result);
                if (!step.plan_suffix.empty())
                {
                    const auto cost = search_result.plan ? static_cast<ygg::float_t>(search_result.plan->get_length()) : ygg::float_t(1);
                    step.state_transition = runir::datasets::StateGraphEdgeLabel { step.plan_suffix.front().label, cost };
                }
                return std::vector<Step> { std::move(step) };
            }
        }

        if (search_result.status == tyr::planning::SearchStatus::OUT_OF_TIME)
            return std::vector<Step> { Step(detail::ModuleProgramOutcome::OUT_OF_TIME, context) };
        if (search_result.status == tyr::planning::SearchStatus::OUT_OF_STATES || search_result.status == tyr::planning::SearchStatus::OUT_OF_MEMORY)
            return std::vector<Step> { Step(detail::ModuleProgramOutcome::OUT_OF_STATES, context) };

        auto eval_context = context;
        if (eval_context.restore_caller())
            return std::vector<Step> { Step(detail::ModuleProgramOutcome::RESTORED_CALLER, std::move(eval_context)) };

        return std::vector<Step> { Step(detail::ModuleProgramOutcome::NO_APPLICABLE_ACTION, context) };
    }

    std::vector<Step> control_successors(const EvaluationContext<Kind>& context, const ModuleProgramSearchOptions<Kind>& options)
    {
        return control_steps(*m_search_context, context, detail::execution_options(options));
    }

    // The control rule (Sketch/Do) of the source vertex that selects the candidate planning
    // successor, or nullopt (the gap). Load/Call are internal memory steps, never a planning move.
    std::optional<RuleVariantView>
    matching_rule_at(EvaluationContext<Kind>& context, tyr::formalism::planning::GroundActionView action, tyr::planning::StateView<Kind> target_state)
    {
        return matching_rule(context, LabeledNode { action, tyr::planning::Node<Kind>(std::move(target_state), ygg::float_t(0)) });
    }

    std::optional<Successor> apply_at(const EvaluationContext<Kind>& context,
                                      RuleVariantView rule,
                                      std::optional<tyr::formalism::planning::GroundActionView> action = std::nullopt,
                                      std::optional<tyr::planning::StateView<Kind>> target_state = std::nullopt)
    {
        std::optional<LabeledNode> candidate;
        if (action && target_state)
            candidate = LabeledNode { *action, tyr::planning::Node<Kind>(*target_state, ygg::float_t(0)) };
        return apply(context, rule, std::move(candidate));
    }

private:
    void ensure_modules(const EvaluationContext<Kind>& context)
    {
        if (m_environment.get_modules().empty())
            m_environment.set_modules(std::vector<ModuleView> { context.get_module() });
    }

    std::optional<RuleVariantView> matching_rule(EvaluationContext<Kind>& context, const LabeledNode& candidate)
    {
        ensure_modules(context);
        for (const auto& transition : context.get_module().get_memory_transitions())
            for (auto rule : transition)
                if (selects(rule, context, candidate))
                    return rule;
        return std::nullopt;
    }

    std::optional<Successor> apply(const EvaluationContext<Kind>& context, RuleVariantView rule, std::optional<LabeledNode> candidate = std::nullopt)
    {
        ensure_modules(context);
        const auto successors = candidate ? std::vector<LabeledNode> { *candidate } : std::vector<LabeledNode> {};
        const auto step = step_for(rule, context, successors);
        return step ? to_successor(*step) : std::nullopt;
    }
    // Apply every rule of the given kinds to the vertex, in module order, keeping the steps that
    // fire. The `if constexpr` fold selects the kinds at compile time, so no runtime rule tag exists.
    template<typename... Kinds>
    std::vector<Step> collect_steps(const EvaluationContext<Kind>& context, const std::vector<LabeledNode>& successors)
    {
        std::vector<Step> result;
        for (const auto& transition : context.get_module().get_memory_transitions())
            for (auto rule_variant : transition)
                ygg::visit(
                    [&](auto rule)
                    {
                        using R = std::decay_t<decltype(rule)>;
                        if constexpr ((matches_rule_kind_v<R, Kinds> || ...))
                            if (auto step = make_step(rule, rule_variant, context, successors))
                                result.push_back(std::move(*step));
                    },
                    rule_variant.get_variant());
        return result;
    }

    std::optional<Step> step_for(RuleVariantView rule_variant, const EvaluationContext<Kind>& context, const std::vector<LabeledNode>& successors)
    {
        return ygg::visit([&](auto rule) { return make_step(rule, rule_variant, context, successors); }, rule_variant.get_variant());
    }

    // Apply one rule to a copy of `context` via the executor's `execute_*`, packaging the outcome as
    // a step. Returns nullopt when the rule does not fire at all (source/conditions mismatch, or a
    // planning rule with no selected successor); a fired rule yields an APPLIED step (with the
    // planning move attached for Sketch/Do) or a terminal step (empty Load denotation, malformed
    // call, no applicable action / restored caller for an exhausted Do).
    template<typename R>
    std::optional<Step> make_step(R rule, RuleVariantView rule_variant, const EvaluationContext<Kind>& context, const std::vector<LabeledNode>& successors)
    {
        if constexpr (is_load_rule_view_v<R>)
        {
            auto target = context;
            const auto status = detail::execute_load(rule, target, m_environment);
            if (status == detail::RuleExecutionStatus::NOT_APPLICABLE)
                return std::nullopt;
            if (status == detail::RuleExecutionStatus::EMPTY_DENOTATION)
                return terminal(detail::ModuleProgramOutcome::FAILURE, context, rule_variant);
            return applied(std::move(target), rule_variant);
        }
        else if constexpr (std::same_as<R, RuleView<DoTag>>)
        {
            const auto source_state = context.get_state();
            auto target = context;
            const auto status = detail::execute_do(rule, target, m_environment, successors);
            if (status == detail::RuleExecutionStatus::NOT_APPLICABLE)
                return std::nullopt;
            if (status == detail::RuleExecutionStatus::NO_APPLICABLE_ACTION)
            {
                auto restored = context;
                if (restored.restore_caller())
                    return Step(detail::ModuleProgramOutcome::RESTORED_CALLER, std::move(restored));
                return Step(detail::ModuleProgramOutcome::NO_APPLICABLE_ACTION, context);
            }
            return planning_step(std::move(target), source_state, successors, rule_variant);
        }
        else if constexpr (std::same_as<R, RuleView<CallTag>>)
        {
            auto target = context;
            const auto status = detail::execute_call(rule, target, m_environment);
            if (status == detail::RuleExecutionStatus::NOT_APPLICABLE)
                return std::nullopt;
            if (status == detail::RuleExecutionStatus::MALFORMED_CALL)
                return Step(detail::ModuleProgramOutcome::MALFORMED_CALL, context);
            return applied(std::move(target), rule_variant);
        }
        else if constexpr (std::same_as<R, RuleView<SketchTag>>)
        {
            const auto source_state = context.get_state();
            auto target = context;
            const auto status = detail::execute_sketch(rule, target, m_environment, successors);
            if (status != detail::RuleExecutionStatus::APPLIED)
                return std::nullopt;
            return planning_step(std::move(target), source_state, successors, rule_variant);
        }
        else
        {
            static_assert(ygg::dependent_false<R>::value, "unhandled rule kind in SuccessorExpander::make_step");
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

    // An APPLIED control step that may have moved the planning state. When the state changed, record
    // the planning action (from the matching generator successor) and its unit cost; a memory-only
    // move leaves the transition empty.
    static Step planning_step(EvaluationContext<Kind> context,
                              const tyr::planning::StateView<Kind>& source_state,
                              const std::vector<LabeledNode>& successors,
                              RuleVariantView rule)
    {
        auto step = applied(std::move(context), rule);
        const auto target_index = step.context.get_state().get_index();
        if (source_state.get_index() == target_index)
            return step;
        for (const auto& successor : successors)
            if (successor.node.get_state().get_index() == target_index)
            {
                step.plan_suffix.push_back(successor);
                step.state_transition = runir::datasets::StateGraphEdgeLabel { successor.label, ygg::float_t(1) };
                break;
            }
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
                if constexpr (is_load_rule_view_v<R>)
                    return InternalMemoryState(step.context.get_memory_state());
                else if constexpr (std::same_as<R, RuleView<DoTag>> || std::same_as<R, RuleView<CallTag>> || std::same_as<R, RuleView<SketchTag>>)
                    return ExternalMemoryState(step.context.get_memory_state());
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
                else if constexpr (is_load_rule_view_v<R> || std::same_as<R, RuleView<CallTag>>)
                    return false;
                else
                    static_assert(ygg::dependent_false<R>::value, "unhandled rule kind in SuccessorExpander::selects");
            },
            rule.get_variant());
    }

    const runir::datasets::TaskSearchContext<Kind>* m_search_context;
    tyr::planning::ConjunctiveGoalStrategy<Kind> m_goal_strategy;
    tyr::planning::StateView<Kind> m_initial_state;
    bool m_static_goal_satisfied;
    EvaluationEnvironment<Kind> m_environment;
};

}  // namespace runir::kr::ps::ext

#endif
