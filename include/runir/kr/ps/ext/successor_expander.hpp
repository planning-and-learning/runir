#ifndef RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_EXT_SUCCESSOR_EXPANDER_HPP_

#include "runir/datasets/state_graph.hpp"
#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/action_rule.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/ext/program_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/ps/ext/transient_execution_storage.hpp"
#include "runir/kr/task_context.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <tyr/formalism/planning/action_view.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>
#include <variant>
#include <vector>
#include <yggdrasil/containers/variant.hpp>

namespace runir::kr::ps::ext
{

/// Returned choices must not outlive this expander.
/// With TransientExecutionStorage, returned states and steps must not outlive it either.
template<tyr::TaskKind Kind, typename ExecutionStorage = InternedExecutionStorage<Kind>>
class SuccessorExpander
{
public:
    SuccessorExpander(runir::kr::TaskContextPtr<Kind> task_context, ProgramView program) :
        m_task_context(task_context ? std::move(task_context) : throw std::invalid_argument("SuccessorExpander requires a task context.")),
        m_program(program),
        m_storage(m_task_context, m_program),
        m_environment(*m_task_context, m_program),
        m_action_rule_evaluator(m_task_context->search_context->task)
    {
        if (&m_program.get_context() != m_task_context->domain_context->ext_repository.get())
            throw std::invalid_argument("SuccessorExpander requires a program from the domain context repository.");
    }

    const auto& get_task_context() const noexcept { return m_task_context; }

    template<StoredProgramStateConcept<Kind> S>
    auto view(const S& state) const
    {
        return m_storage.view(state);
    }

    /// Create the entry module with empty registers and arguments, and no caller.
    auto initial_state(const tyr::planning::StateView<Kind>& state)
    {
        validate_planning_state(state);
        return m_storage.initial_state(state);
    }

    /// Intern selected output states and, in CHOICE mode, sources with Choose obligations.
    template<ProgramStateViewConcept<Kind> S>
    ProgramStateView<Kind> materialize(const S& state)
    {
        validate_source(state);
        return m_storage.materialize(state);
    }

    /// Emit a program step or a compact Choose obligation in natural rule and binding order,
    /// except that sketch rules with effects are emitted together, binding-major, after all other rules.
    /// Return true on exhaustion; emit returning false or stop returning true ends enumeration.
    /// Count applied successors and caller returns, not Choice descriptors or failure markers.
    /// Callbacks may materialize the source, but must not generate successors or apply choices. Apply choices after enumeration.
    template<ProgramStateViewConcept<Kind> S, typename Emit, typename Stop>
    bool for_each_successor(S state, ProgramSearchStatistics& statistics, Emit&& emit, Stop&& stop)
    {
        using Step = detail::ProgramStep<Kind, decltype(m_storage.retain(state))>;
        if (stop())
            return false;
        validate_source(state);
        const auto planning_state = state.get_state();
        m_environment.get_dl_caches().clear(false);
        bool emitted = false;
        const auto emit_expansion = [&](auto expansion)
        {
            emitted = true;
            if constexpr (std::same_as<decltype(expansion), Step>)
                statistics.num_generated += expansion.status == detail::ProgramOutcome::APPLIED || expansion.status == detail::ProgramOutcome::RESTORED_CALLER;
            return emit(std::move(expansion));
        };
        m_sketch_rules.clear();
        for (const auto transition : state.get_module_state().get_module().get_memory_transitions())
            for (const auto rule : transition)
            {
                if (stop())
                    return false;
                if (!ygg::visit([&](auto concrete) { return emit_rule(concrete, rule, state, planning_state, emit_expansion, stop); }, rule.get_variant()))
                    return false;
            }
        if (!emit_sketch_successors(state, planning_state, emit_expansion, stop))
            return false;
        if (stop())
            return false;
        return emitted || emit_expansion(fallback(state));
    }

    /// Apply the current admitted binding without advancing its cursor; an exhausted choice reports FAILURE.
    template<runir::kr::dl::CategoryTag Category, ProgramStateViewConcept<Kind> S>
    auto apply_choice(S state, const detail::Choice<Category>& choice, ProgramSearchStatistics& statistics)
    {
        validate_source(state);
        auto step = choice_step<Category>(state, choice);
        statistics.num_generated += step.status == detail::ProgramOutcome::APPLIED;
        return step;
    }

    /// Find the first rule that admits this planning successor; control-only rules do not match planning actions.
    std::optional<RuleVariantView> matching_rule(ProgramStateView<Kind> state, const tyr::planning::LabeledNode<tyr::planning::StateView<Kind>>& candidate)
    {
        validate_source(state);
        validate_planning_state(candidate.node.get_state());
        const auto planning_state = state.get_state();
        m_environment.get_dl_caches().clear(false);
        m_environment.get_dl_target_caches().clear(false);
        for (const auto& transition : state.get_module_state().get_module().get_memory_transitions())
            for (auto rule : transition)
                if (ygg::visit([&](auto concrete) { return matches(concrete, state, planning_state, candidate); }, rule.get_variant()))
                    return rule;
        return std::nullopt;
    }

    /// Apply one rule, using a supplied planning successor for Do, Action, or a Sketch with effects.
    /// Load, Choose, Call, and empty-effect Sketch rules derive their own control transition.
    std::optional<detail::ProgramStep<Kind>> apply(ProgramStateView<Kind> state,
                                                   RuleVariantView rule,
                                                   std::optional<tyr::planning::LabeledNode<tyr::planning::StateView<Kind>>> candidate = std::nullopt)
    {
        validate_source(state);
        if (candidate)
            validate_planning_state(candidate->node.get_state());
        const auto planning_state = state.get_state();
        m_environment.get_dl_caches().clear(false);
        m_environment.get_dl_target_caches().clear(false);
        return ygg::visit([&](auto concrete) { return apply_rule(concrete, rule, state, planning_state, candidate); }, rule.get_variant());
    }

private:
    std::vector<ygg::uint_t> m_order_scores;
    std::vector<size_t> m_order_indices;

    // Validate borrowed views before evaluating features or modifying the execution repository.
    template<tyr::planning::StateViewConcept<Kind> S>
    void validate_planning_state(const S& state) const
    {
        if constexpr (requires { state.get_state_repository(); })
        {
            if (state.get_state_repository().get() != m_task_context->search_context->state_repository.get())
                throw std::invalid_argument("SuccessorExpander requires a planning state from the selected task's state repository.");
        }
        else if (&state.get_task() != m_task_context->search_context->task.get())
            throw std::invalid_argument("SuccessorExpander requires a planning state from the selected task.");
    }

    template<ProgramStateViewConcept<Kind> S>
    void validate_source(S state) const
    {
        if (&state.get_context() != m_task_context->execution_repository.get())
            throw std::invalid_argument("SuccessorExpander requires an execution state from the selected task.");
        const auto program = state.get_program();
        if (&program.get_context() != &m_program.get_context() || program.get_index() != m_program.get_index())
            throw std::invalid_argument("SuccessorExpander requires an execution state from the selected program.");
    }

    template<ProgramStateViewConcept<Kind> S>
    auto copy_module(const S& state)
    {
        auto result = m_storage.module_();
        *result = state.get_module_state().get_data();
        return result;
    }

    static bool arguments_match(ModuleView callee, runir::kr::dl::semantics::CallArgumentsView arguments)
    {
        return arguments.template get<runir::kr::dl::ConceptTag>().size() == callee.template get_arguments<runir::kr::dl::ConceptTag>().size()
               && arguments.template get<runir::kr::dl::RoleTag>().size() == callee.template get_arguments<runir::kr::dl::RoleTag>().size()
               && arguments.template get<runir::kr::dl::BooleanTag>().size() == callee.template get_arguments<runir::kr::dl::BooleanTag>().size()
               && arguments.template get<runir::kr::dl::NumericalTag>().size() == callee.template get_arguments<runir::kr::dl::NumericalTag>().size();
    }

    template<typename ModuleData>
    void set_empty_registers(ModuleData& target, ModuleView module_)
    {
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(m_task_context->dl_builder);
        registers->concept_values.resize(module_.template get_registers<runir::kr::dl::ConceptTag>().size());
        registers->role_values.resize(module_.template get_registers<runir::kr::dl::RoleTag>().size());
        m_storage.set_registers(target, m_storage.registers(*registers));
    }

    // Rule admission and argument/effect evaluation share the environment's reusable denotation caches.
    template<RuleKind RuleKindT, typename C, ProgramStateViewConcept<Kind> S>
    static bool has_current_source(ygg::View<ygg::Index<Rule<RuleKindT>>, C> rule, S state)
    {
        return rule.get_source().get_index() == state.get_module_state().get_memory_state().get_index();
    }

    template<RuleKind RuleKindT, typename C, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool rule_is_applicable(ygg::View<ygg::Index<Rule<RuleKindT>>, C> rule, S state, const PS& planning_state)
    {
        if (!has_current_source(rule, state))
            return false;
        auto state_context = m_environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
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

    template<typename C, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    auto& evaluate_do_arguments(ygg::View<ygg::Index<Rule<DoTag>>, C> rule, S state, const PS& planning_state)
    {
        const auto arguments = rule.get_action_arguments();
        auto& denotations = m_environment.prepare_do_argument_denotations();
        auto state_context = m_environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
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

    template<typename C, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool do_effects_match(ygg::View<ygg::Index<Rule<DoTag>>, C> rule, S state, const PS& planning_state, const PS& target_state)
    {
        auto transition = m_environment.make_dl_transition_context(planning_state,
                                                                   target_state,
                                                                   state.get_module_state().get_arguments(),
                                                                   state.get_module_state().get_registers(),
                                                                   state.get_module_state().get_registers());
        return is_compatible_with(rule, transition);
    }

    template<typename C, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool do_rule_matches(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                         S state,
                         const PS& planning_state,
                         tyr::formalism::planning::ActionBindingView action,
                         const PS& target_state)
    {
        if (!rule_is_applicable(rule, state, planning_state))
            return false;
        const auto& denotations = evaluate_do_arguments(rule, state, planning_state);
        return action_matches_do_arguments(rule, action, denotations) && do_effects_match(rule, state, planning_state, target_state);
    }

    template<typename C, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool sketch_rule_matches_state(ygg::View<ygg::Index<Rule<SketchTag>>, C> rule, S state, const PS& planning_state, const PS& target_state)
    {
        if (!has_current_source(rule, state))
            return false;
        auto transition = m_environment.make_dl_transition_context(planning_state,
                                                                   target_state,
                                                                   state.get_module_state().get_arguments(),
                                                                   state.get_module_state().get_registers(),
                                                                   state.get_module_state().get_registers());
        return is_compatible_with(rule, transition);
    }

    template<typename C, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    auto evaluate_call_arguments(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, S state, const PS& planning_state)
    {
        auto result = checkout<runir::kr::dl::semantics::CallArguments>(m_task_context->dl_builder);
        auto state_context = m_environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        rule.for_each_call_argument(
            [&]<typename FeatureTag, typename FC>(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, FC> argument)
            {
                const auto denotation = evaluate(argument.get_expression(), state_context, *m_task_context->dl_denotation_repository);
                if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
                    result->concept_arguments.push_back(denotation.get_index());
                else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
                    result->role_arguments.push_back(denotation.get_index());
                else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
                    result->boolean_arguments.push_back(denotation.get_index());
                else
                    result->numerical_arguments.push_back(denotation.get_index());
            });
        return get_or_create(*m_task_context->dl_denotation_repository, *result).first;
    }

    template<BindingRuleKind RuleKindT, typename Value, ProgramStateViewConcept<Kind> S>
    auto bound_registers(RuleView<RuleKindT> rule, S state, const Value& value, ygg::Data<runir::kr::dl::semantics::RegisterValues>& registers)
    {
        registers.concept_values = state.get_module_state().get_registers().get_data().concept_values;
        registers.role_values = state.get_module_state().get_registers().get_data().role_values;
        apply_binding(rule, value, registers);
        return m_storage.registers(registers);
    }

    template<BindingRuleKind RuleKindT,
             ProgramStateViewConcept<Kind> S,
             tyr::planning::StateViewConcept<Kind> PS,
             runir::kr::dl::semantics::RegisterValuesViewConcept R>
    bool binding_effects_match(RuleView<RuleKindT> rule, S state, const PS& planning_state, R registers)
    {
        if (rule.get_effects().empty())
            return true;
        m_environment.get_dl_target_caches().clear(false);
        auto transition = m_environment.make_dl_transition_context(planning_state,
                                                                   planning_state,
                                                                   state.get_module_state().get_arguments(),
                                                                   state.get_module_state().get_registers(),
                                                                   registers);
        return is_compatible_with(rule, transition);
    }

    /// Bind a register and move memory while preserving the planning state and caller stack.
    template<runir::kr::dl::CategoryTag Category, typename ChoiceType, ProgramStateViewConcept<Kind> S>
    auto choice_step(S state, const ChoiceType& choice)
    {
        if (choice.exhausted())
        {
            auto failure = make_step(detail::ProgramOutcome::FAILURE, m_storage.retain(state));
            failure.rule = choice.rule;
            return failure;
        }
        const auto rule = choice.rule.get_variant().template get<ygg::Index<Rule<ChooseTag<Category>>>>();
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(m_task_context->dl_builder);
        auto target = copy_module(state);
        m_storage.set_registers(*target, bound_registers(rule, state, choice.current(), *registers));
        m_storage.set_memory_state(*target, rule.get_target());
        return applied(m_storage.store(std::move(target), m_storage.call_stack(state)), choice.rule);
    }

    template<runir::kr::dl::CategoryTag Category, typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit_rule(RuleView<LoadTag<Category>> rule, RuleVariantView rule_variant, S state, const PS& planning_state, Emit&& emit, Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, planning_state))
            return true;
        auto state_context = m_environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        const auto denotation = evaluate(rule.get_feature(), state_context);
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(m_task_context->dl_builder);
        for (const auto value : denotation)
        {
            if (stop())
                return false;
            const auto target_registers = bound_registers(rule, state, value, *registers);
            if (!binding_effects_match(rule, state, planning_state, target_registers))
                continue;
            auto target = copy_module(state);
            m_storage.set_registers(*target, target_registers);
            m_storage.set_memory_state(*target, rule.get_target());
            if (!emit(applied(m_storage.store(std::move(target), m_storage.call_stack(state)), rule_variant)))
                return false;
        }
        return true;
    }

    template<runir::kr::dl::ConceptOrRoleTag Category>
    auto make_choice(RuleVariantView rule, runir::kr::dl::semantics::DenotationView<Category> denotation)
    {
        if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
            return detail::Choice<Category>(rule, denotation, m_concept_bindings);
        else
            return detail::Choice<Category>(rule, denotation, m_role_bindings);
    }

    /// Ordering is evaluated after binding, and only rearranges the admitted values.
    template<runir::kr::dl::CategoryTag Category, typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit_choice(RuleView<ChooseTag<Category>> rule,
                     RuleVariantView rule_variant,
                     S state,
                     const PS& planning_state,
                     runir::kr::dl::semantics::DenotationView<Category> denotation,
                     Emit&& emit,
                     Stop&& stop)
    {
        auto choice = make_choice(rule_variant, denotation);
        if (stop())
            return false;
        if (rule.get_order().empty() || !choice.has_alternatives())
            return emit(std::move(choice));

        // Flat scratch buffers are reused across choices; only ordered values survive in the DFS frame.
        m_order_scores.clear();
        m_order_indices.clear();
        const auto width = rule.get_order().size();
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(m_task_context->dl_builder);
        for (const auto value : choice.bindings())
        {
            if (stop())
                return false;
            const auto target_registers = bound_registers(rule, state, value, *registers);
            m_environment.get_dl_target_caches().clear(false);
            auto transition = m_environment.make_dl_transition_context(planning_state,
                                                                       planning_state,
                                                                       state.get_module_state().get_arguments(),
                                                                       state.get_module_state().get_registers(),
                                                                       target_registers);
            for (auto term : rule.get_order())
            {
                if (stop())
                    return false;
                m_order_scores.push_back(
                    ygg::visit([&](auto feature) { return ygg::uint_t(evaluate(feature, transition.get_target_context()).get()); }, term.get_feature()));
            }
            m_order_indices.push_back(m_order_indices.size());
        }
        if (stop())
            return false;
        // The original ordinal is the final key, giving stable ties without sort scratch allocations.
        std::sort(m_order_indices.begin(),
                  m_order_indices.end(),
                  [&](size_t lhs, size_t rhs)
                  {
                      size_t column = 0;
                      for (auto term : rule.get_order())
                      {
                          const auto left = m_order_scores[lhs * width + column];
                          const auto right = m_order_scores[rhs * width + column++];
                          if (left != right)
                              return term.get_direction() == OrderDirection::MIN ? left < right : left > right;
                      }
                      return lhs < rhs;
                  });
        if (stop())
            return false;
        // Convert the sorted source indices into an in-place permutation.
        for (size_t i = 0; i < m_order_indices.size(); ++i)
        {
            auto current = i;
            while (m_order_indices[current] != i)
            {
                const auto next = m_order_indices[current];
                std::swap(choice.bindings()[current], choice.bindings()[next]);
                m_order_indices[current] = current;
                current = next;
            }
            m_order_indices[current] = current;
        }
        return !stop() && emit(std::move(choice));
    }

    template<runir::kr::dl::CategoryTag Category, typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit_rule(RuleView<ChooseTag<Category>> rule, RuleVariantView rule_variant, S state, const PS& planning_state, Emit&& emit, Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, planning_state))
            return true;
        auto state_context = m_environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        if (rule.get_effects().empty())
        {
            const auto denotation = evaluate(rule.get_feature(), state_context);
            return emit_choice(rule, rule_variant, state, planning_state, denotation, emit, stop);
        }
        const auto denotation = evaluate(rule.get_feature(), state_context);
        if (stop())
            return false;

        auto admitted = m_task_context->dl_builder.template get_builder<runir::kr::dl::semantics::Denotation<Category>>(denotation.get_data().num_objects);
        auto registers = checkout<runir::kr::dl::semantics::RegisterValues>(m_task_context->dl_builder);
        for (const auto value : denotation)
        {
            if (stop())
                return false;
            const auto target_registers = bound_registers(rule, state, value, *registers);
            if (!binding_effects_match(rule, state, planning_state, target_registers))
                continue;
            if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
                admitted->get().set(ygg::uint_t(value.get_index()));
            else
                admitted->get(value.first.get_index()).set(ygg::uint_t(value.second.get_index()));
        }
        if (stop())
            return false;
        return emit_choice(
            rule,
            rule_variant,
            state,
            planning_state,
            runir::kr::dl::semantics::detail::materialize_denotation(admitted, m_task_context->dl_builder, m_environment.get_dl_caches().get_repository(false))
                .first,
            emit,
            stop);
    }

    template<typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit_rule(RuleView<DoTag> rule, RuleVariantView rule_variant, S state, const PS& planning_state, Emit&& emit, Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, planning_state))
            return true;
        const auto& denotations = evaluate_do_arguments(rule, state, planning_state);
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
                const auto candidate = m_storage.successor(planning_state, binding);
                m_environment.get_dl_target_caches().clear(false);
                if (!do_effects_match(rule, state, planning_state, candidate.node.get_state()))
                    return true;
                return emit(planning_step(state, candidate, rule_variant, rule.get_target()));
            };
            return search.successor_generator->for_each_applicable_action_binding(tyr::planning::Node<PS>(planning_state, 0), action, std::ref(visit));
        }
        return true;
    }

    template<ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS, typename N>
    void check_action_effects(RuleView<ActionTag> rule, S state, const PS& planning_state, const N& candidate, std::span<const ygg::uint_t> tuple)
    {
        m_environment.get_dl_target_caches().clear(false);
        auto transition = m_environment.make_dl_transition_context(planning_state,
                                                                   candidate.node.get_state(),
                                                                   state.get_module_state().get_arguments(),
                                                                   state.get_module_state().get_registers(),
                                                                   state.get_module_state().get_registers());
        for (const auto effect : rule.get_effects())
            if (!is_compatible_with(effect, transition))
                detail::action_rule_contract_error(rule, planning_state, tuple, "offered transition violates declared effects");
    }

    template<typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit_rule(RuleView<ActionTag> rule, RuleVariantView rule_variant, S state, const PS& planning_state, Emit&& emit, Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, planning_state))
            return true;
        auto state_context = m_environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        const auto query = evaluate(rule.get_query_feature(), state_context);
        m_action_rule_evaluator.action(rule, planning_state, query.arity());
        for (std::size_t i = 0; i < query.size(); ++i)
        {
            if (stop())
                return false;
            const auto tuple = query[i];
            const auto binding = m_action_rule_evaluator.applicable_binding(rule, planning_state, tuple);
            const auto candidate = m_storage.successor(planning_state, binding);
            check_action_effects(rule, state, planning_state, candidate, tuple);
            if (!emit(planning_step(state, candidate, rule_variant, rule.get_target())))
                return false;
        }
        return true;
    }

    template<typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit_rule(RuleView<SketchTag> rule, RuleVariantView rule_variant, S state, const PS& planning_state, Emit&& emit, Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, planning_state))
            return true;
        // Rules with effects share one successor enumeration in emit_sketch_successors.
        if (!rule.get_effects().empty())
        {
            m_sketch_rules.emplace_back(rule, rule_variant);
            return true;
        }
        if (stop())
            return false;
        auto target = copy_module(state);
        m_storage.set_memory_state(*target, rule.get_target());
        return emit(applied(m_storage.store(std::move(target), m_storage.call_stack(state)), rule_variant));
    }

    /// Generate each planning successor once and test every collected sketch rule against it,
    /// emitting one step per matching (successor, rule) pair in binding-major, rule-minor order.
    template<typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit_sketch_successors(S state, const PS& planning_state, Emit&& emit, Stop&& stop)
    {
        if (m_sketch_rules.empty())
            return true;
        auto& generator = *m_task_context->search_context->successor_generator;
        const auto visit = [&](tyr::formalism::planning::ActionBindingView binding)
        {
            if (stop())
                return false;
            const auto candidate = m_storage.successor(planning_state, binding);
            m_environment.get_dl_target_caches().clear(false);
            for (const auto& [sketch_rule, rule_variant] : m_sketch_rules)
            {
                if (!sketch_rule_matches_state(sketch_rule, state, planning_state, candidate.node.get_state()))
                    continue;
                if (!emit(planning_step(state, candidate, rule_variant, sketch_rule.get_target())))
                    return false;
            }
            return true;
        };
        return generator.for_each_applicable_action_binding(tyr::planning::Node<PS>(planning_state, 0), std::ref(visit));
    }

    template<typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit_rule(RuleView<CallTag> rule, RuleVariantView rule_variant, S state, const PS& planning_state, Emit&& emit, Stop&& stop)
    {
        if (!rule_is_applicable(rule, state, planning_state))
            return true;
        auto arguments = evaluate_call_arguments(rule, state, planning_state);
        const auto callee = m_program.find_module(rule.get_callee().get_index());
        if (stop())
            return false;
        if (!callee || !arguments_match(*callee, arguments))
            return emit(make_step(detail::ProgramOutcome::MALFORMED_CALL, m_storage.retain(state)));

        auto target = copy_module(state);
        auto caller = m_storage.save_caller(state, rule.get_target());
        m_storage.set_module(*target, *callee);
        m_storage.set_memory_state(*target, callee->get_entry_memory_state());
        set_empty_registers(*target, *callee);
        m_storage.set_arguments(*target, arguments);
        return emit(applied(m_storage.store(std::move(target), std::move(caller)), rule_variant));
    }

    template<BindingRuleKind RuleKindT, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS, typename N>
    bool matches(RuleView<RuleKindT>, S, const PS&, const N&)
    {
        return false;
    }

    template<ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS, typename N>
    bool matches(RuleView<CallTag>, S, const PS&, const N&)
    {
        return false;
    }

    template<ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS, typename N>
    bool matches(RuleView<DoTag> rule, S state, const PS& planning_state, const N& candidate)
    {
        return do_rule_matches(rule, state, planning_state, candidate.label, candidate.node.get_state());
    }

    template<ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS, typename N>
    bool matches(RuleView<SketchTag> rule, S state, const PS& planning_state, const N& candidate)
    {
        return !rule.get_effects().empty() && sketch_rule_matches_state(rule, state, planning_state, candidate.node.get_state());
    }

    template<ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS, typename N>
    bool matches(RuleView<ActionTag> rule, S state, const PS& planning_state, const N& candidate)
    {
        if (!rule_is_applicable(rule, state, planning_state) || candidate.label.get_relation().get_name() != rule.get_action_name())
            return false;
        auto state_context = m_environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        const auto query = evaluate(rule.get_query_feature(), state_context);
        m_action_rule_evaluator.action(rule, planning_state, query.arity());
        m_action_tuple.clear();
        for (const auto object : candidate.label.get_objects())
            m_action_tuple.push_back(ygg::uint_t(object.get_index()));
        if (m_action_tuple.size() != query.arity() || !query.contains(std::span<const ygg::uint_t>(m_action_tuple)))
            return false;
        m_action_rule_evaluator.applicable_binding(rule, planning_state, m_action_tuple);
        check_action_effects(rule, state, planning_state, candidate, m_action_tuple);
        return true;
    }

    template<RuleKind RuleKindT, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS, typename N>
        requires(BindingRuleKind<RuleKindT> || std::same_as<RuleKindT, CallTag>)
    auto apply_rule(RuleView<RuleKindT> rule, RuleVariantView rule_variant, S state, const PS& planning_state, const std::optional<N>&)
    {
        auto result = std::optional<detail::ProgramStep<Kind, decltype(m_storage.retain(state))>> {};
        emit_rule(
            rule,
            rule_variant,
            state,
            planning_state,
            [&](auto expansion)
            {
                if constexpr (ChooseRuleView<decltype(rule)>)
                    result = choice_step<typename RuleKindT::Category>(state, expansion);
                else
                    result = std::move(expansion);
                return false;
            },
            [] { return false; });
        return result;
    }

    template<typename Tag, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS, typename N>
        requires(std::same_as<Tag, DoTag> || std::same_as<Tag, ActionTag> || std::same_as<Tag, SketchTag>)
    auto apply_rule(RuleView<Tag> rule, RuleVariantView rule_variant, S state, const PS& planning_state, const std::optional<N>& candidate)
    {
        using Result = std::optional<detail::ProgramStep<Kind, decltype(m_storage.retain(state))>>;
        if constexpr (std::same_as<Tag, SketchTag>)
            if (rule.get_effects().empty())
            {
                if (!rule_is_applicable(rule, state, planning_state))
                    return Result {};
                auto target = copy_module(state);
                m_storage.set_memory_state(*target, rule.get_target());
                return Result(applied(m_storage.store(std::move(target), m_storage.call_stack(state)), rule_variant));
            }
        if (!candidate || !matches(rule, state, planning_state, *candidate))
            return Result {};
        return Result(planning_step(state, *candidate, rule_variant, rule.get_target()));
    }

    template<StoredProgramStateConcept<Kind> S>
    detail::ProgramStep<Kind, S> make_step(detail::ProgramOutcome status, S state)
    {
        return detail::ProgramStep<Kind, S>(status, std::move(state), m_task_context);
    }

    /// With no emitted rule outcome, return to the caller or report an open top-level state.
    /// Restore caller control and bindings while retaining the planning state reached by the callee.
    template<ProgramStateViewConcept<Kind> S>
    auto fallback(S state)
    {
        if (const auto caller = state.get_call_stack())
        {
            auto target = copy_module(state);
            m_storage.set_module(*target, caller->get_module());
            m_storage.set_memory_state(*target, caller->get_return_memory_state());
            m_storage.set_registers(*target, caller->get_registers());
            m_storage.set_arguments(*target, caller->get_arguments());
            return make_step(detail::ProgramOutcome::RESTORED_CALLER, m_storage.store(std::move(target), m_storage.caller(*caller)));
        }
        return make_step(detail::ProgramOutcome::NO_APPLICABLE_ACTION, m_storage.retain(state));
    }

    template<StoredProgramStateConcept<Kind> S>
    detail::ProgramStep<Kind, S> applied(S state, RuleVariantView rule)
    {
        auto step = make_step(detail::ProgramOutcome::APPLIED, std::move(state));
        step.rule = rule;
        return step;
    }

    template<ProgramStateViewConcept<Kind> S, typename N>
    auto planning_step(S state, const N& successor, RuleVariantView rule, MemoryStateView memory_state)
    {
        auto target = copy_module(state);
        m_storage.set_planning_state(*target, successor);
        m_storage.set_memory_state(*target, memory_state);
        auto step = applied(m_storage.store(std::move(target), m_storage.call_stack(state)), rule);
        if constexpr (requires { successor.pack(); })
            step.planning_successor = successor.pack();
        step.state_transition = datasets::StateGraphEdgeLabel { successor.label, ygg::float_t(1) };
        return step;
    }

    runir::kr::TaskContextPtr<Kind> m_task_context;
    ProgramView m_program;
    ExecutionStorage m_storage;
    EvaluationEnvironment<Kind> m_environment;
    ygg::UniqueObjectPool<runir::kr::dl::semantics::DenotationElementViewList<runir::kr::dl::ConceptTag>> m_concept_bindings;
    ygg::UniqueObjectPool<runir::kr::dl::semantics::DenotationElementViewList<runir::kr::dl::RoleTag>> m_role_bindings;
    detail::ActionRuleEvaluator<Kind> m_action_rule_evaluator;
    std::vector<ygg::uint_t> m_action_tuple;
    std::vector<std::pair<RuleView<SketchTag>, RuleVariantView>> m_sketch_rules;
};

template<tyr::TaskKind Kind>
using TransientSuccessorExpander = SuccessorExpander<Kind, TransientExecutionStorage<Kind>>;

#ifndef RUNIR_HEADER_INSTANTIATION

extern template class SuccessorExpander<tyr::GroundTag>;
extern template class SuccessorExpander<tyr::LiftedTag>;

extern template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag>::materialize<ProgramStateView<tyr::GroundTag>>(const ProgramStateView<tyr::GroundTag>&);

extern template auto
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

extern template auto
SuccessorExpander<tyr::GroundTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::GroundTag>>(ProgramStateView<tyr::GroundTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

extern template ProgramStateView<tyr::GroundTag>
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::materialize<BuilderProgramStateView<tyr::GroundTag>>(
    const BuilderProgramStateView<tyr::GroundTag>&);

extern template auto
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

extern template auto
SuccessorExpander<tyr::GroundTag, TransientExecutionStorage<tyr::GroundTag>>::apply_choice<runir::kr::dl::RoleTag, BuilderProgramStateView<tyr::GroundTag>>(
    BuilderProgramStateView<tyr::GroundTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

extern template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag>::materialize<ProgramStateView<tyr::LiftedTag>>(const ProgramStateView<tyr::LiftedTag>&);

extern template auto
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::ConceptTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                             const detail::Choice<runir::kr::dl::ConceptTag>&,
                                                                                                             ProgramSearchStatistics&);

extern template auto
SuccessorExpander<tyr::LiftedTag>::apply_choice<runir::kr::dl::RoleTag, ProgramStateView<tyr::LiftedTag>>(ProgramStateView<tyr::LiftedTag>,
                                                                                                          const detail::Choice<runir::kr::dl::RoleTag>&,
                                                                                                          ProgramSearchStatistics&);

extern template ProgramStateView<tyr::LiftedTag>
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::materialize<BuilderProgramStateView<tyr::LiftedTag>>(
    const BuilderProgramStateView<tyr::LiftedTag>&);

extern template auto
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::ConceptTag, BuilderProgramStateView<tyr::LiftedTag>>(
    BuilderProgramStateView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::ConceptTag>&,
    ProgramSearchStatistics&);

extern template auto
SuccessorExpander<tyr::LiftedTag, TransientExecutionStorage<tyr::LiftedTag>>::apply_choice<runir::kr::dl::RoleTag, BuilderProgramStateView<tyr::LiftedTag>>(
    BuilderProgramStateView<tyr::LiftedTag>,
    const detail::Choice<runir::kr::dl::RoleTag>&,
    ProgramSearchStatistics&);

#endif

}  // namespace runir::kr::ps::ext

#endif
