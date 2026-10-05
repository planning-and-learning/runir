#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CHOOSE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_CHOOSE_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/binding.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <algorithm>
#include <concepts>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext::detail
{

struct ChooseRuleWorkspace
{
    ygg::UniqueObjectPool<runir::kr::dl::semantics::DenotationElementViewList<runir::kr::dl::ConceptTag>> concept_bindings;
    ygg::UniqueObjectPool<runir::kr::dl::semantics::DenotationElementViewList<runir::kr::dl::RoleTag>> role_bindings;
    std::vector<ygg::uint_t> order_scores;
    std::vector<size_t> order_indices;
};

template<tyr::TaskKind Kind, runir::kr::dl::ConceptOrRoleTag Category>
class ChooseRuleEvaluator
{
    RuleView<ChooseTag<Category>> m_rule;
    RuleVariantView m_variant;

public:
    using RuleTag = ChooseTag<Category>;
    ChooseRuleEvaluator(RuleView<ChooseTag<Category>> rule, RuleVariantView variant) : m_rule(rule), m_variant(variant) {}
    auto get_rule() const noexcept { return m_rule; }
    auto get_variant() const noexcept { return m_variant; }

    /// Resume a retained choice after child search; emit() only enumerates and prepares its alternatives.
    /// Bind a register and move memory while preserving the planning state and caller stack.
    template<ExecutionStorageConcept<Kind> Storage, ExecutionStateViewConcept<Storage> State, tyr::planning::StateViewConcept<Kind> PlanningState>
    auto choice_step(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context, State state, const Choice<Category>& choice) const
    {
        if (choice.exhausted())
        {
            auto failure = detail::make_step<Kind, Storage>(ProgramOutcome::FAILURE, context.storage.retain(state), context.task_context);
            failure.rule = choice.rule;
            return failure;
        }
        const auto rule = m_rule;
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(context.task_context->dl_builder);
        const auto module_ = state.get_module_state();
        auto target = context.storage.store(context.planning_state,
                                            module_.get_module(),
                                            rule.get_target(),
                                            bound_registers(rule, module_.get_registers(), choice.current(), *registers, context.storage),
                                            module_.get_arguments(),
                                            state.get_call_stack());
        return detail::applied<Kind, Storage>(std::move(target), choice.rule, context.task_context);
    }

private:
    auto make_choice(ChooseRuleWorkspace& workspace, RuleVariantView rule, runir::kr::dl::semantics::DenotationView<Category> denotation) const
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return detail::Choice<Category>(rule, denotation, workspace.concept_bindings);
        else
            return detail::Choice<Category>(rule, denotation, workspace.role_bindings);
    }
    /// Ordering is evaluated after binding, and only rearranges the admitted values.
    template<ExecutionStorageConcept<Kind> Storage,
             EmitConcept<Choice<Category>> Emit,
             StopConcept Stop,
             ProgramStateViewConcept<Kind> State,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    bool emit_choice(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context,
                     ChooseRuleWorkspace& workspace,
                     RuleView<ChooseTag<Category>> rule,
                     RuleVariantView rule_variant,
                     State state,
                     const PlanningState& planning_state,
                     runir::kr::dl::semantics::DenotationView<Category> denotation,
                     Emit&& emit,
                     Stop&& stop) const
    {
        auto choice = make_choice(workspace, rule_variant, denotation);
        if (stop())
            return false;
        if (!rule.get_effects().empty())
        {
            auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(context.task_context->dl_builder);
            auto& bindings = choice.bindings();
            auto admitted = bindings.begin();
            for (const auto value : bindings)
            {
                if (stop())
                    return false;
                const auto target_registers = bound_registers(rule, state.get_module_state().get_registers(), value, *registers, context.storage);
                if (binding_effects_match(rule, state, planning_state, target_registers, context.environment))
                    *admitted++ = value;
            }
            bindings.erase(admitted, bindings.end());
            if (stop())
                return false;
        }
        if (rule.get_order().empty() || !choice.has_alternatives())
            return emit(std::move(choice));

        // Flat scratch buffers are reused across choices; only ordered values survive in the DFS frame.
        workspace.order_scores.clear();
        workspace.order_indices.clear();
        const auto width = rule.get_order().size();
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(context.task_context->dl_builder);
        for (const auto value : choice.bindings())
        {
            if (stop())
                return false;
            const auto target_registers = bound_registers(rule, state.get_module_state().get_registers(), value, *registers, context.storage);
            context.environment.reset_target();
            auto transition = context.environment.make_dl_transition_context(planning_state,
                                                                             planning_state,
                                                                             state.get_module_state().get_arguments(),
                                                                             state.get_module_state().get_registers(),
                                                                             target_registers);
            for (auto term : rule.get_order())
            {
                if (stop())
                    return false;
                workspace.order_scores.push_back(
                    ygg::visit([&](auto feature) { return ygg::uint_t(evaluate<Kind>(feature, transition.get_target_context()).get()); }, term.get_feature()));
            }
            workspace.order_indices.push_back(workspace.order_indices.size());
        }
        if (stop())
            return false;
        // The original ordinal is the final key, giving stable ties without sort scratch allocations.
        std::sort(workspace.order_indices.begin(),
                  workspace.order_indices.end(),
                  [&](size_t lhs, size_t rhs)
                  {
                      size_t column = 0;
                      for (auto term : rule.get_order())
                      {
                          const auto left = workspace.order_scores[lhs * width + column];
                          const auto right = workspace.order_scores[rhs * width + column++];
                          if (left != right)
                              return term.get_direction() == OrderDirection::MIN ? left < right : left > right;
                      }
                      return lhs < rhs;
                  });
        if (stop())
            return false;
        // Convert the sorted source indices into an in-place permutation.
        for (size_t i = 0; i < workspace.order_indices.size(); ++i)
        {
            auto current = i;
            while (workspace.order_indices[current] != i)
            {
                const auto next = workspace.order_indices[current];
                std::swap(choice.bindings()[current], choice.bindings()[next]);
                workspace.order_indices[current] = current;
                current = next;
            }
            workspace.order_indices[current] = current;
        }
        return !stop() && emit(std::move(choice));
    }

public:
    template<ExecutionStorageConcept<Kind> Storage,
             EmitConcept<Choice<Category>> Emit,
             StopConcept Stop,
             ExecutionStateViewConcept<Storage> State,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    bool emit(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context, State state, Emit&& emit, Stop&& stop) const
    {
        const auto& planning_state = context.planning_state;
        auto& workspace = context.choose_workspace;
        const auto rule = m_rule;
        const auto rule_variant = m_variant;
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment))
            return true;
        auto state_context = context.make_dl_context(state);
        const auto denotation = evaluate<Kind>(rule.get_feature(), state_context);
        return emit_choice(context, workspace, rule, rule_variant, state, planning_state, denotation, emit, stop);
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
