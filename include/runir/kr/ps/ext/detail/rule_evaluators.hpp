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
#include <span>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <tyr/planning/node.hpp>
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
    std::vector<size_t> m_sketch_rules;

    template<RuleKind Tag>
    auto prepare(RuleView<Tag> rule, RuleVariantView variant)
    {
        if constexpr (std::same_as<Tag, ActionTag>)
        {
            for (const auto action : m_task_context->search_context->task->get_task().get_domain().get_actions())
                if (action.get_name().str() == rule.get_action_name())
                    return ActionRuleEvaluator<Kind>(rule, variant, action);
            auto message = std::ostringstream {};
            message << "Action rule " << ygg::uint_t(rule.get_index()) << " ('" << variant.get_symbol().str() << "') for '" << rule.get_action_name().str()
                    << "': action schema does not exist in the task";
            throw ActionRuleContractError(message.str());
        }
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
                return std::visit(function, std::as_const(m_rules[found->second]));
        }
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
             ExecutionStateViewConcept<Storage> State,
             typename Emit,
             StopConcept Stop,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    bool
    emit_rule(const Evaluator& evaluator, RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context, State state, Emit&& emit, Stop&& stop)
    {
        using Tag = typename Evaluator::RuleTag;
        if constexpr (ChooseRuleView<RuleView<Tag>>)
            static_assert(EmittingRuleEvaluatorConcept<Evaluator,
                                                       ExtFamilyTag,
                                                       Kind,
                                                       RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>,
                                                       State,
                                                       Choice<typename Tag::Category>,
                                                       Emit,
                                                       Stop>);
        else
            static_assert(EmittingRuleEvaluatorConcept<Evaluator,
                                                       ExtFamilyTag,
                                                       Kind,
                                                       RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>,
                                                       State,
                                                       ProgramStep<Kind, Storage>,
                                                       Emit,
                                                       Stop>);
        return evaluator.emit(context, state, emit, stop);
    }

    template<typename Evaluator,
             ExecutionStorageConcept<Kind> Storage,
             ProgramStateViewConcept<Kind> State,
             tyr::planning::StateViewConcept<Kind> PlanningState,
             ygg::formalism::RelationBindingViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>, tyr::formalism::ObjectTag> Binding>
    bool matches(const Evaluator& evaluator,
                 RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context,
                 State state,
                 const tyr::planning::LabeledNode<Kind, PlanningState, Binding>& candidate)
    {
        using Tag = typename Evaluator::RuleTag;
        if constexpr (std::same_as<Tag, ActionTag> || std::same_as<Tag, DoTag> || std::same_as<Tag, SketchTag>)
        {
            static_assert(MatchingRuleEvaluatorConcept<Evaluator,
                                                       ExtFamilyTag,
                                                       Kind,
                                                       RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>,
                                                       State,
                                                       tyr::planning::LabeledNode<Kind, PlanningState, Binding>>);
            return evaluator.matches(context, state, candidate);
        }
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
                    const auto source = std::visit([](const auto& evaluator) { return evaluator.get_rule().get_source().get_index(); }, m_rules[found->second]);
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

    template<ExecutionStorageConcept<Kind> Storage, tyr::planning::StateViewConcept<Kind> PlanningState>
    auto make_context(Storage& storage, PlanningState planning_state)
    {
        return runir::kr::ps::RuleEvaluationContext<runir::kr::ExtFamilyTag, Kind, Storage, PlanningState> {
            m_task_context, storage, m_environment, m_do, m_choose, std::move(planning_state)
        };
    }

    template<ExecutionStorageConcept<Kind> Storage,
             ExecutionStateViewConcept<Storage> State,
             typename Emit,
             StopConcept Stop,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    bool for_each_successor(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context, State state, Emit&& emit, Stop&& stop)
    {
        const auto& planning_state = context.planning_state;
        m_sketch_rules.clear();
        for (const auto slot : source_rules(state.get_module_state().get_module(), state.get_module_state().get_memory_state()))
        {
            const auto& evaluator = m_rules[slot];
            if (stop())
                return false;
            if (!std::visit(
                    [&]<typename Concrete>(const Concrete& concrete)
                    {
                        if constexpr (std::same_as<Concrete, SketchRuleEvaluator<Kind>>)
                            if (!concrete.get_rule().get_effects().empty())
                            {
                                if (ext::rule_is_applicable(concrete.get_rule(), state, planning_state, context.environment))
                                    m_sketch_rules.push_back(slot);
                                return true;
                            }
                        return emit_rule(concrete, context, state, emit, stop);
                    },
                    evaluator))
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
            const auto borrowed_candidate = tyr::planning::LabeledNode { binding, candidate };
            context.environment.reset_target();
            auto label = std::optional<tyr::formalism::planning::ActionBindingView> {};
            for (const auto slot : m_sketch_rules)
            {
                if (stop())
                    return false;
                const auto& evaluator = std::get<SketchRuleEvaluator<Kind>>(m_rules[slot]);
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
    std::optional<RuleVariantView> matching_rule(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context,
                                                 State state,
                                                 const tyr::planning::LabeledNode<Kind, PlanningState>& candidate)
    {
        for (const auto slot : source_rules(state.get_module_state().get_module(), state.get_module_state().get_memory_state()))
        {
            const auto& evaluator = m_rules[slot];
            const auto matched = std::visit(
                [&](const auto& concrete) -> std::optional<RuleVariantView>
                {
                    if (matches(concrete, context, state, candidate))
                        return concrete.get_variant();
                    return std::nullopt;
                },
                evaluator);
            if (matched)
                return matched;
        }
        return std::nullopt;
    }

    template<ExecutionStorageConcept<Kind> Storage, ExecutionStateViewConcept<Storage> State, tyr::planning::StateViewConcept<Kind> PlanningState>
    auto apply(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context,
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
                emit_rule(
                    evaluator,
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
    auto apply_choice(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context, State state, const Choice<Category>& choice)
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
