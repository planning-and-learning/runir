#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATORS_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATORS_HPP_

#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/action.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/call.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/choose.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/do.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/load.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/sketch.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <concepts>
#include <functional>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <tyr/planning/node.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace runir::kr::ps::ext::detail
{

/// Prepared rule instances and reusable scratch for one task/program pair.
/// Records stay in place after construction; pending Choices borrow this owner's pools.
template<tyr::TaskKind Kind, runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy = runir::kr::dl::semantics::DefaultEvaluationPolicy<ExtFamilyTag, Kind>>
class RuleEvaluators
{
    using Concept = runir::kr::dl::ConceptTag;
    using Role = runir::kr::dl::RoleTag;
    using Evaluator = std::variant<LoadRuleEvaluator<Kind, Concept>,
                                   LoadRuleEvaluator<Kind, Role>,
                                   SketchRuleEvaluator<Kind>,
                                   DoRuleEvaluator<Kind>,
                                   CallRuleEvaluator<Kind>,
                                   ChooseRuleEvaluator<Kind, Concept>,
                                   ChooseRuleEvaluator<Kind, Role>,
                                   ActionRuleEvaluator<Kind>>;
    runir::kr::TaskContextPtr<Kind> m_task_context;
    ProgramView m_program;
    EvaluationEnvironment<Kind, EvaluationPolicy> m_environment;
    std::vector<Evaluator> m_rules;
    ChooseRuleWorkspace m_choose;
    DoRuleWorkspace m_do;
    std::vector<SketchRuleEvaluator<Kind>> m_sketch_rules;

    template<RuleKind Tag>
    auto prepare(RuleView<Tag> rule, RuleVariantView variant)
    {
        if constexpr (std::same_as<Tag, ActionTag> || std::same_as<Tag, DoTag>)
        {
            for (const auto action : m_task_context->search_context->task->get_task().get_domain().get_actions())
                if (action.get_name().str() == rule.get_action_name())
                {
                    if constexpr (std::same_as<Tag, ActionTag>)
                        return ActionRuleEvaluator<Kind>(rule, variant, action);
                    else
                        return DoRuleEvaluator<Kind>(rule, variant, action);
                }
            if constexpr (std::same_as<Tag, ActionTag>)
                throw std::logic_error("Action rule: action schema does not exist in the domain.");
            else
                throw std::logic_error("Do rule: action schema does not exist in the domain.");
        }
        else if constexpr (std::same_as<Tag, CallTag>)
            return CallRuleEvaluator<Kind>(rule, variant, m_program);
        else if constexpr (std::same_as<Tag, SketchTag>)
            return SketchRuleEvaluator<Kind>(rule, variant);
        else if constexpr (ChooseRuleView<RuleView<Tag>>)
            return ChooseRuleEvaluator<Kind, RuleCategoryFor<Tag>>(rule, variant);
        else
            return LoadRuleEvaluator<Kind, RuleCategoryFor<Tag>>(rule, variant);
    }

    auto source_rules(ModuleView module_, MemoryStateView memory_state) const
    {
        const auto modules = m_program.get_modules();
        const auto belongs_to_program = &module_.get_context() == &m_program.get_context() && std::ranges::find(modules, module_) != modules.end();
        return module_.get_memory_transitions() | std::views::join
               | std::views::filter(
                   [belongs_to_program, memory_state](RuleVariantView rule) {
                       return belongs_to_program
                              && ygg::visit([memory_state](auto concrete) { return concrete.get_source() == memory_state; }, rule.get_variant());
                   });
    }

    auto find_rule(RuleVariantView rule)
    {
        // Repository indices alone do not identify a rule supplied to apply().
        if (&rule.get_context() != &m_program.get_context())
            return m_rules.end();
        // ponytail: Linear lookup keeps dispatch simple; index it only if profiling shows a bottleneck.
        return std::ranges::find_if(m_rules,
                                    [rule](const auto& evaluator)
                                    { return std::visit([rule](const auto& concrete) { return concrete.get_variant() == rule; }, evaluator); });
    }

    template<typename Function>
    decltype(auto) with_rule(RuleVariantView rule, Function&& function)
    {
        if (const auto found = find_rule(rule); found != m_rules.end())
            return std::visit(function, std::as_const(*found));
        // Direct application historically accepts rules outside the prepared program.
        return ygg::visit(
            [&](auto concrete) -> decltype(auto)
            {
                const auto evaluator = prepare(concrete, rule);
                return function(evaluator);
            },
            rule.get_variant());
    }

    template<typename Evaluator,
             ExecutionStorageConcept<Kind> Storage,
             ProgramStateViewConcept<Kind> State,
             tyr::planning::StateViewConcept<Kind> PlanningState,
             ygg::formalism::RelationBindingViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>, tyr::formalism::ObjectTag> Binding>
    bool matches(const Evaluator& evaluator,
                 RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context,
                 State state,
                 const tyr::planning::LabeledNode<Kind, PlanningState, Binding>& candidate)
    {
        using Tag = typename Evaluator::RuleTag;
        if constexpr (std::same_as<Tag, ActionTag> || std::same_as<Tag, DoTag> || std::same_as<Tag, SketchTag>)
            return evaluator.matches(context, state, candidate);
        else
            return false;
    }

public:
    RuleEvaluators(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program) :
        m_task_context(task_context ? std::move(task_context) : throw std::invalid_argument("RuleEvaluators requires a task context.")),
        m_program(program),
        m_environment(*m_task_context, m_program)
    {
        if (&program.get_context() != m_task_context->domain_context->ext_repository.get())
            throw std::invalid_argument("RuleEvaluators requires a program from the domain context repository.");
        size_t count = 0;
        for (const auto module_ : program.get_modules())
            for (const auto transition : module_.get_memory_transitions())
                count += transition.size();
        m_rules.reserve(count);
        m_sketch_rules.reserve(count);
        for (const auto module_ : program.get_modules())
            for (const auto transition : module_.get_memory_transitions())
                for (const auto rule : transition)
                    if (find_rule(rule) == m_rules.end())
                        m_rules.push_back(ygg::visit([&](auto concrete) -> Evaluator { return prepare(concrete, rule); }, rule.get_variant()));
    }

    auto& get_environment() noexcept { return m_environment; }

    template<ExecutionStorageConcept<Kind> Storage, tyr::planning::StateViewConcept<Kind> PlanningState>
    auto make_context(Storage& storage, PlanningState planning_state)
    {
        return runir::kr::ps::RuleEvaluationContext<runir::kr::ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy> {
            m_task_context, storage, m_environment, m_do, m_choose, std::move(planning_state)
        };
    }

    template<ExecutionStorageConcept<Kind> Storage,
             ExecutionStateViewConcept<Storage> State,
             typename Emit,
             StopConcept Stop,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    bool for_each_successor(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context, State state, Emit&& emit, Stop&& stop)
    {
        const auto& planning_state = context.planning_state;
        m_sketch_rules.clear();
        for (const auto rule : source_rules(state.get_module_state().get_module(), state.get_module_state().get_memory_state()))
        {
            if (stop())
                return false;
            if (!with_rule(rule,
                           [&]<typename Concrete>(const Concrete& concrete)
                           {
                               if constexpr (std::same_as<Concrete, SketchRuleEvaluator<Kind>>)
                                   if (!concrete.get_rule().get_effects().empty())
                                   {
                                       if (ext::rule_is_applicable(concrete.get_rule(), state, planning_state, context.environment))
                                           m_sketch_rules.push_back(concrete);
                                       return true;
                                   }
                               return concrete.emit(context, state, emit, stop);
                           }))
                return false;
        }
        if (m_sketch_rules.empty())
            return true;
        // Each successor is generated once for the complete effectful Sketch batch.
        auto& generator = *m_task_context->search_context->successor_generator;
        const auto visit = [&](tyr::planning::BorrowedActionBindingView<Kind> binding)
        {
            if (stop())
                return false;
            const auto candidate = context.storage.successor(planning_state, binding);
            const auto borrowed_candidate =
                tyr::planning::LabeledNode<Kind, PlanningState, tyr::planning::BorrowedActionBindingView<Kind>> { binding, candidate };
            context.environment.reset_target();
            auto label = std::optional<tyr::formalism::planning::ActionBindingView> {};
            for (const auto& evaluator : m_sketch_rules)
            {
                if (stop())
                    return false;
                if (!matches(evaluator, context, state, borrowed_candidate))
                    continue;
                if (stop())
                    return false;
                if (!label)
                    label = generator.materialize_action_binding(binding);
                const auto labeled = tyr::planning::LabeledNode<Kind, PlanningState> { *label, candidate };
                if (!emit(detail::planning_step(context.storage,
                                                state,
                                                labeled,
                                                evaluator.get_variant(),
                                                evaluator.get_rule().get_target(),
                                                context.task_context)))
                    return false;
            }
            return true;
        };
        return generator.for_each_borrowed_applicable_action_binding(tyr::planning::Node<Kind, PlanningState>(planning_state, 0), std::ref(visit));
    }

    template<ExecutionStorageConcept<Kind> Storage, ProgramStateViewConcept<Kind> State, tyr::planning::StateViewConcept<Kind> PlanningState>
    std::optional<RuleVariantView> matching_rule(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context,
                                                 State state,
                                                 const tyr::planning::LabeledNode<Kind, PlanningState>& candidate)
    {
        for (const auto rule : source_rules(state.get_module_state().get_module(), state.get_module_state().get_memory_state()))
        {
            const auto matched = with_rule(rule,
                                           [&](const auto& concrete) -> std::optional<RuleVariantView>
                                           {
                                               if (matches(concrete, context, state, candidate))
                                                   return concrete.get_variant();
                                               return std::nullopt;
                                           });
            if (matched)
                return matched;
        }
        return std::nullopt;
    }

    template<ExecutionStorageConcept<Kind> Storage, ExecutionStateViewConcept<Storage> State, tyr::planning::StateViewConcept<Kind> PlanningState>
    auto apply(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context,
               State state,
               RuleVariantView rule,
               const std::optional<tyr::planning::LabeledNode<Kind, PlanningState>>& candidate)
    {
        using Result = std::optional<ProgramStep<Kind, Storage>>;
        return with_rule(
            rule,
            [&]<typename Concrete>(const Concrete& evaluator) -> Result
            {
                using Tag = typename Concrete::RuleTag;
                if constexpr (!BindingRuleKind<Tag> && !std::same_as<Tag, CallTag>)
                {
                    if (!std::same_as<Tag, SketchTag> || !evaluator.get_rule().get_effects().empty())
                    {
                        if (!candidate || !matches(evaluator, context, state, *candidate))
                            return {};
                        return detail::planning_step(context.storage, state, *candidate, rule, evaluator.get_rule().get_target(), context.task_context);
                    }
                }
                auto result = Result {};
                evaluator.emit(
                    context,
                    state,
                    [&](auto expansion)
                    {
                        if constexpr (ChooseRuleView<RuleView<Tag>>)
                            result = evaluator.choice_step(context, state, expansion);
                        else
                            result = std::move(expansion);
                        return false;
                    },
                    [] { return false; });
                return result;
            });
    }

    template<runir::kr::dl::ConceptOrRoleTag Category,
             ExecutionStorageConcept<Kind> Storage,
             ExecutionStateViewConcept<Storage> State,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    auto apply_choice(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState, EvaluationPolicy>& context, State state, const Choice<Category>& choice)
    {
        using Step = ProgramStep<Kind, Storage>;
        return with_rule(choice.rule,
                         [&]<typename Concrete>(const Concrete& evaluator) -> Step
                         {
                             if constexpr (std::same_as<Concrete, ChooseRuleEvaluator<Kind, Category>>)
                                 return evaluator.choice_step(context, state, choice);
                             else
                                 throw std::invalid_argument("Choice requires a rule of its binding category.");
                         });
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
