#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATORS_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATORS_HPP_

#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/action.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/call.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/choose.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/do.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/load.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/sketch.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <concepts>
#include <functional>
#include <optional>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace runir::kr::ps::ext::detail
{

/// Prepared rule instances and reusable scratch for one task/program pair.
/// Records stay in place after construction; pending Choices borrow this owner's pools.
template<tyr::TaskKind Kind>
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
    struct MemoryRules
    {
        ygg::Index<Module> module_;
        ygg::Index<MemoryState> memory_state;
        size_t begin;
        size_t count;
    };

    runir::kr::TaskContextPtr<Kind> m_task_context;
    ProgramView m_program;
    EvaluationEnvironment<Kind> m_environment;
    std::vector<Evaluator> m_rules;
    std::vector<size_t> m_schedule;
    std::vector<MemoryRules> m_memory_rules;
    std::vector<std::pair<ygg::Index<runir::kr::ps::Rule<runir::kr::ExtFamilyTag>>, size_t>> m_lookup;
    ChooseRuleWorkspace m_choose;
    DoRuleWorkspace m_do;
    ActionRuleWorkspace<Kind> m_action;
    std::vector<size_t> m_sketch_rules;

    template<RuleKind Tag>
    auto prepare(RuleView<Tag> rule, RuleVariantView variant)
    {
        if constexpr (std::same_as<Tag, ActionTag>)
            return ActionRuleEvaluator<Kind>(rule, variant, *m_task_context->search_context->task);
        else if constexpr (std::same_as<Tag, CallTag>)
            return CallRuleEvaluator<Kind>(rule, variant, m_program);
        else if constexpr (std::same_as<Tag, DoTag>)
            return DoRuleEvaluator<Kind>(rule, variant, *m_task_context->search_context->task);
        else if constexpr (std::same_as<Tag, SketchTag>)
            return SketchRuleEvaluator<Kind>(rule, variant);
        else if constexpr (ChooseRuleView<RuleView<Tag>>)
            return ChooseRuleEvaluator<Kind, typename Tag::Category>(rule, variant);
        else
            return LoadRuleEvaluator<Kind, typename Tag::Category>(rule, variant);
    }

    auto source_rules(ModuleView module_, MemoryStateView memory_state)
    {
        const auto key = std::pair(module_.get_index(), memory_state.get_index());
        const auto found = std::lower_bound(m_memory_rules.begin(),
                                            m_memory_rules.end(),
                                            key,
                                            [](const auto& entry, const auto& value) { return std::pair(entry.module_, entry.memory_state) < value; });
        if (found == m_memory_rules.end() || found->module_ != key.first || found->memory_state != key.second)
            return std::span<const size_t> {};
        return std::span<const size_t>(m_schedule).subspan(found->begin, found->count);
    }

    template<typename Function>
    decltype(auto) with_rule(RuleVariantView rule, Function&& function)
    {
        // Repository indices alone do not identify a rule supplied to apply().
        if (&rule.get_context() == &m_program.get_context())
        {
            const auto found =
                std::lower_bound(m_lookup.begin(), m_lookup.end(), rule.get_index(), [](const auto& entry, auto index) { return entry.first < index; });
            if (found != m_lookup.end() && found->first == rule.get_index())
                return std::visit(function, m_rules[found->second]);
        }
        // Direct application historically accepts rules outside the prepared program.
        return ygg::visit(
            [&](auto concrete) -> decltype(auto)
            {
                auto evaluator = prepare(concrete, rule);
                return function(evaluator);
            },
            rule.get_variant());
    }

    template<typename RuleEvaluator, typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS, typename Emit, typename Stop>
    bool emit_rule(RuleEvaluator& evaluator, Context& context, S state, const PS& planning_state, Emit&& emit, Stop&& stop)
    {
        using Tag = typename RuleEvaluator::RuleTag;
        if constexpr (std::same_as<Tag, ActionTag>)
            return evaluator.emit(context, state, planning_state, emit, stop, m_action);
        else if constexpr (std::same_as<Tag, DoTag>)
            return evaluator.emit(context, state, planning_state, emit, stop, m_do);
        else if constexpr (ChooseRuleView<RuleView<Tag>>)
            return evaluator.emit(context, state, planning_state, emit, stop, m_choose);
        else if constexpr (std::same_as<Tag, SketchTag>)
        {
            if (!ext::rule_is_applicable(evaluator.rule(), state, planning_state, context.environment))
                return true;
            if (stop())
                return false;
            return emit(evaluator.control_step(context, state));
        }
        else
            return evaluator.emit(context, state, planning_state, emit, stop);
    }

    template<typename RuleEvaluator, typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool matches(RuleEvaluator& evaluator, Context& context, S state, const PS& planning_state, const tyr::planning::LabeledNode<PS>& candidate)
    {
        using Tag = typename RuleEvaluator::RuleTag;
        if constexpr (std::same_as<Tag, ActionTag>)
            return evaluator.matches(context, state, planning_state, candidate, m_action);
        else if constexpr (std::same_as<Tag, DoTag>)
            return evaluator.matches(context, state, planning_state, candidate, m_do);
        else if constexpr (std::same_as<Tag, SketchTag>)
            return evaluator.matches(context, state, planning_state, candidate);
        else
            return false;
    }

public:
    RuleEvaluators(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program) :
        m_task_context(task_context ? std::move(task_context) : throw std::invalid_argument("RuleEvaluators requires a task context.")),
        m_program(program),
        m_environment(*m_task_context, m_program),
        m_action(m_task_context->search_context->task)
    {
        if (&program.get_context() != m_task_context->domain_context->ext_repository.get())
            throw std::invalid_argument("RuleEvaluators requires a program from the domain context repository.");
        auto modules = std::vector<ModuleView> {};
        modules.reserve(program.get_modules().size());
        for (const auto module_ : program.get_modules())
            modules.push_back(module_);
        std::sort(modules.begin(), modules.end(), [](const auto& lhs, const auto& rhs) { return lhs.get_index() < rhs.get_index(); });
        modules.erase(std::unique(modules.begin(), modules.end()), modules.end());
        size_t count = 0;
        for (const auto module_ : modules)
            for (const auto transition : module_.get_memory_transitions())
                count += transition.size();
        m_lookup.reserve(count);
        for (const auto module_ : modules)
            for (const auto transition : module_.get_memory_transitions())
                for (const auto rule : transition)
                    m_lookup.emplace_back(rule.get_index(), count);
        std::sort(m_lookup.begin(), m_lookup.end());
        m_lookup.erase(std::unique(m_lookup.begin(), m_lookup.end()), m_lookup.end());
        m_rules.reserve(m_lookup.size());
        m_schedule.reserve(count);
        m_memory_rules.reserve(count);
        m_sketch_rules.reserve(count);
        // The ordinal preserves declaration order when grouping rules by their source memory.
        auto scheduled = std::vector<std::tuple<ygg::Index<Module>, ygg::Index<MemoryState>, size_t, size_t>> {};
        scheduled.reserve(count);
        for (const auto module_ : modules)
            for (const auto transition : module_.get_memory_transitions())
                for (const auto rule : transition)
                {
                    auto found =
                        std::lower_bound(m_lookup.begin(), m_lookup.end(), rule.get_index(), [](const auto& entry, auto index) { return entry.first < index; });
                    if (found->second == count)
                    {
                        found->second = m_rules.size();
                        m_rules.push_back(ygg::visit([&](auto concrete) -> Evaluator { return prepare(concrete, rule); }, rule.get_variant()));
                    }
                    const auto source = std::visit([](const auto& evaluator) { return evaluator.rule().get_source().get_index(); }, m_rules[found->second]);
                    scheduled.emplace_back(module_.get_index(), source, scheduled.size(), found->second);
                }
        std::sort(scheduled.begin(), scheduled.end());
        for (const auto& [module_, memory_state, ordinal, slot] : scheduled)
        {
            if (m_memory_rules.empty() || m_memory_rules.back().module_ != module_ || m_memory_rules.back().memory_state != memory_state)
                m_memory_rules.push_back({ module_, memory_state, m_schedule.size(), 0 });
            m_schedule.push_back(slot);
            ++m_memory_rules.back().count;
        }
    }

    auto& get_environment() noexcept { return m_environment; }

    template<typename Context, ProgramStateViewConcept<Kind> S, typename Emit, typename Stop>
    bool for_each_successor(Context& context, S state, Emit&& emit, Stop&& stop)
    {
        const auto planning_state = state.get_state();
        m_sketch_rules.clear();
        for (const auto slot : source_rules(state.get_module_state().get_module(), state.get_module_state().get_memory_state()))
        {
            auto& evaluator = m_rules[slot];
            if (stop())
                return false;
            if (!std::visit(
                    [&](auto& concrete)
                    {
                        if constexpr (std::same_as<typename std::remove_cvref_t<decltype(concrete)>::RuleTag, SketchTag>)
                            if (!concrete.rule().get_effects().empty())
                            {
                                if (ext::rule_is_applicable(concrete.rule(), state, planning_state, context.environment))
                                    m_sketch_rules.push_back(slot);
                                return true;
                            }
                        return emit_rule(concrete, context, state, planning_state, emit, stop);
                    },
                    evaluator))
                return false;
        }
        if (m_sketch_rules.empty())
            return true;
        // Each successor is generated once for the complete effectful Sketch batch.
        auto& generator = *m_task_context->search_context->successor_generator;
        const auto visit = [&](tyr::formalism::planning::ActionBindingView binding)
        {
            if (stop())
                return false;
            const auto candidate = context.storage.successor(planning_state, binding);
            context.environment.reset_target();
            for (const auto slot : m_sketch_rules)
            {
                auto& evaluator = std::get<SketchRuleEvaluator<Kind>>(m_rules[slot]);
                if (evaluator.matches(context, state, planning_state, candidate)
                    && !emit(
                        detail::planning_step(context.storage, state, candidate, evaluator.variant(), evaluator.rule().get_target(), context.task_context)))
                    return false;
            }
            return true;
        };
        return generator.for_each_applicable_action_binding(tyr::planning::Node<std::remove_cvref_t<decltype(planning_state)>>(planning_state, 0),
                                                            std::ref(visit));
    }

    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    std::optional<RuleVariantView> matching_rule(Context& context, S state, const tyr::planning::LabeledNode<PS>& candidate)
    {
        const auto planning_state = state.get_state();
        for (const auto slot : source_rules(state.get_module_state().get_module(), state.get_module_state().get_memory_state()))
        {
            auto& evaluator = m_rules[slot];
            const auto matched = std::visit(
                [&](auto& concrete) -> std::optional<RuleVariantView>
                {
                    if (matches(concrete, context, state, planning_state, candidate))
                        return concrete.variant();
                    return std::nullopt;
                },
                evaluator);
            if (matched)
                return matched;
        }
        return std::nullopt;
    }

    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    auto apply(Context& context, S state, RuleVariantView rule, const std::optional<tyr::planning::LabeledNode<PS>>& candidate)
    {
        using Result = std::optional<ProgramStep<Kind, typename Context::StorageType>>;
        const auto planning_state = state.get_state();
        return with_rule(rule,
                         [&](auto& evaluator) -> Result
                         {
                             using Tag = typename std::remove_cvref_t<decltype(evaluator)>::RuleTag;
                             if constexpr (BindingRuleKind<Tag> || std::same_as<Tag, CallTag>)
                             {
                                 auto result = Result {};
                                 emit_rule(
                                     evaluator,
                                     context,
                                     state,
                                     planning_state,
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
                             }
                             else
                             {
                                 if constexpr (std::same_as<Tag, SketchTag>)
                                     if (evaluator.rule().get_effects().empty())
                                     {
                                         if (!ext::rule_is_applicable(evaluator.rule(), state, planning_state, context.environment))
                                             return {};
                                         return evaluator.control_step(context, state);
                                     }
                                 if (!candidate || !matches(evaluator, context, state, planning_state, *candidate))
                                     return {};
                                 return detail::planning_step(context.storage, state, *candidate, rule, evaluator.rule().get_target(), context.task_context);
                             }
                         });
    }

    template<runir::kr::dl::ConceptOrRoleTag Category, typename Context, ProgramStateViewConcept<Kind> S>
    auto apply_choice(Context& context, S state, const Choice<Category>& choice)
    {
        using Step = ProgramStep<Kind, typename Context::StorageType>;
        return with_rule(choice.rule,
                         [&](auto& evaluator) -> Step
                         {
                             if constexpr (std::same_as<std::remove_cvref_t<decltype(evaluator)>, ChooseRuleEvaluator<Kind, Category>>)
                                 return evaluator.choice_step(context, state, choice);
                             else
                                 throw std::invalid_argument("Choice requires a rule of its binding category.");
                         });
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
