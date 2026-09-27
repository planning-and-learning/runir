#ifndef RUNIR_KR_PS_ICP_SUCCESSOR_EXPANDER_HPP_
#define RUNIR_KR_PS_ICP_SUCCESSOR_EXPANDER_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/icp/compatibility.hpp"
#include "runir/kr/ps/icp/detail/execution_step.hpp"
#include "runir/kr/ps/icp/evaluation_environment.hpp"

#include <algorithm>
#include <functional>
#include <tyr/formalism/planning/action_view.hpp>
#include <unordered_map>

namespace runir::kr::ps::icp
{

template<tyr::TaskKind Kind>
class SuccessorExpander
{
    using Concept = runir::kr::dl::ConceptTag;
    using Denotation = runir::kr::dl::semantics::Denotation<Concept>;
    using Registers = runir::kr::dl::semantics::RegisterValues;
    using RegistersView = runir::kr::dl::semantics::RegisterValuesView;
    using Transition = runir::kr::ps::dl::TransitionEvaluationContext<IcpFamilyTag, Kind>;
    using Crule = std::pair<RuleView<CruleTag>, RuleVariantView>;
    struct ActionRules
    {
        tyr::formalism::planning::ActionView<tyr::LiftedTag> action;
        std::vector<Crule> rules;
    };
    TaskContextPtr<Kind> m_task;
    ProgramView m_program;
    EvaluationEnvironment<Kind> m_environment;
    std::unordered_map<ygg::uint_t, std::vector<RuleVariantView>> m_rules;
    std::unordered_map<ygg::uint_t, tyr::formalism::planning::ActionView<tyr::LiftedTag>> m_actions;
    std::vector<ActionRules> m_enabled;
    std::vector<Crule> m_matching;
    std::vector<std::vector<size_t>> m_reset_predecessors;
    std::vector<bool> m_changed;
    ygg::Index<Denotation> m_empty;

    void validate(tyr::planning::StateView<Kind> state) const
    {
        if (state.get_state_repository().get() != m_task->search_context->state_repository.get())
            throw std::invalid_argument("ICP requires a planning state from the selected task.");
    }
    void validate(ProgramStateView<Kind> state) const
    {
        if (&state.get_context() != m_task->icp_execution_repository.get() || state.get_program() != m_program)
            throw std::invalid_argument("ICP requires an execution state from the selected task and program.");
    }

    auto intern(ygg::Data<ProgramState<Kind>>& data) { return get_or_create(*m_task->icp_execution_repository, data).first; }
    auto intern(ygg::Data<Histories>& data) { return get_or_create(*m_task->icp_execution_repository, data).first; }

    template<typename Context>
    bool xconditions_match(RuleView<CruleTag> rule, ProgramStateView<Kind> state, tyr::formalism::planning::ActionBindingView binding, Context& context)
    {
        for (const auto condition : rule.get_xconditions())
        {
            const auto object = resolve(condition.get_object_reference(), state.get_registers(), binding);
            if (!object)
                return false;
            const auto contains = evaluate(condition.get_concept_feature(), context).get().test(*object);
            if (contains != (condition.get_operation() == ConditionOperation::BELONGS))
                return false;
        }
        return true;
    }
    bool xeffects_match(RuleView<CruleTag> rule, ProgramStateView<Kind> state, tyr::formalism::planning::ActionBindingView binding, Transition& transition)
    {
        for (const auto effect : rule.get_xeffects())
        {
            const auto object = resolve(effect.get_object_reference(), state.get_registers(), binding);
            if (!object)
                return false;
            const auto before = evaluate(effect.get_concept_feature(), transition.get_source_context()).get().test(*object);
            const auto after = evaluate(effect.get_concept_feature(), transition.get_target_context()).get().test(*object);
            if (effect.get_operation() == EffectOperation::ENTER ? (before || !after) : (!before || after))
                return false;
        }
        return true;
    }
    std::optional<ygg::uint_t> resolve(const ObjectReference& reference, RegistersView registers, tyr::formalism::planning::ActionBindingView binding) const
    {
        return reference.apply(
            [&](auto ref) -> std::optional<ygg::uint_t>
            {
                if constexpr (std::same_as<decltype(ref), ArgumentPosition>)
                    return ygg::uint_t(binding.get_objects().at(ref.value).get_index());
                else
                {
                    const auto reg = ygg::make_view(ref, m_program.get_context().get_dl_repository());
                    const auto object = registers.at(reg.get_identifier());
                    return object ? std::optional(ygg::uint_t(object.value().get_index())) : std::nullopt;
                }
            });
    }

    template<typename Stop>
    std::optional<HistoriesView<Kind>>
    update_histories(ProgramStateView<Kind> state, Transition& transition, std::optional<tyr::formalism::planning::ActionBindingView> binding, Stop&& stop)
    {
        auto histories = state.get_histories().get_data();
        const auto previous = state.get_histories().get_concepts();
        const auto features = m_program.get_module().template get_features<Concept>();
        m_changed.assign(features.size(), false);
        bool progress = !binding;
        for (size_t i = 0; i < features.size(); ++i)
        {
            if (stop())
                return std::nullopt;
            const auto before = evaluate(features[i], transition.get_source_context());
            const auto after = evaluate(features[i], transition.get_target_context());
            auto entered = m_task->dl_builder.template get_builder<Denotation>(before.get_data().num_objects);
            entered->get().copy_from(after.get());
            entered->get() -= before.get();
            if (entered->get().intersects(previous[i].get()))
                return std::nullopt;
            m_changed[i] = before.get() != after.get();
            if (entered->get().any())
            {
                if (binding && !progress)
                    for (size_t j = 0; j < binding->get_relation().get_original_arity(); ++j)
                        if (entered->get().test(ygg::uint_t(binding->get_objects()[j].get_index())))
                        {
                            progress = true;
                            break;
                        }
                entered->get() |= previous[i].get();
                histories.concepts[i] =
                    runir::kr::dl::semantics::detail::materialize_denotation(entered, m_task->dl_builder, *m_task->dl_denotation_repository).first.get_index();
            }
        }
        if (!progress)
            return std::nullopt;
        // Resets follow admission and accumulation, including predecessors which entered in this step.
        for (size_t i = 0; i < m_changed.size(); ++i)
            if (m_changed[i])
                for (const auto predecessor : m_reset_predecessors[i])
                    histories.concepts[predecessor] = m_empty;
        return intern(histories);
    }

    template<typename Rule>
    bool effects_match(Rule rule, Transition& transition)
    {
        for (const auto effect : rule.get_effects())
            if (!is_compatible_with(effect, transition))
                return false;
        return true;
    }

    template<runir::kr::dl::CategoryTag Category, typename Emit, typename Stop>
    bool emit_load(RuleView<LoadTag<Category>> rule, RuleVariantView variant, ProgramStateView<Kind> source, Emit&& emit, Stop&& stop)
    {
        auto context = m_environment.make_dl_context(source);
        const auto denotation = evaluate(rule.get_feature(), context);
        if (denotation.begin() == denotation.end())
        {
            auto step = Step(detail::ProgramOutcome::FAILURE, source, m_task);
            step.rule = variant;
            return emit(std::move(step));
        }
        auto registers = checkout<Registers>(m_task->dl_builder);
        for (const auto value : denotation)
        {
            if (stop())
                return false;
            registers->concept_values = source.get_registers().get_data().concept_values;
            registers->role_values = source.get_registers().get_data().role_values;
            const auto position = ygg::uint_t(rule.get_register().get_identifier());
            if constexpr (std::same_as<Category, Concept>)
                registers->concept_values.at(position) = value.get_index();
            else
                registers->role_values.at(position) = ::cista::pair(value.first.get_index(), value.second.get_index());
            const auto target_registers = get_or_create(*m_task->dl_denotation_repository, *registers).first;
            m_environment.get_dl_target_caches().clear(false);
            auto transition = m_environment.make_dl_transition_context(source.get_state(), source.get_state(), source.get_registers(), target_registers);
            if (!effects_match(rule, transition))
                continue;
            const auto histories = update_histories(source, transition, std::nullopt, stop);
            if (!histories)
                continue;
            auto target = source.get_data();
            target.memory_state = rule.get_target().get_index();
            target.registers = target_registers.get_index();
            target.histories = histories->get_index();
            auto step = Step(detail::ProgramOutcome::APPLIED, intern(target), m_task);
            step.rule = variant;
            if (!emit(std::move(step)))
                return false;
        }
        return !stop();
    }

    template<typename Emit, typename Stop>
    bool emit_crules(tyr::formalism::planning::ActionView<tyr::LiftedTag> action,
                     const std::vector<Crule>& rules,
                     ProgramStateView<Kind> source,
                     Emit&& emit,
                     Stop&& stop)
    {
        auto& search = *m_task->search_context;
        auto context = m_environment.make_dl_context(source);
        const auto visit = [&](tyr::formalism::planning::ActionBindingView binding)
        {
            if (stop())
                return false;
            m_matching.clear();
            for (const auto& rule : rules)
                if (xconditions_match(rule.first, source, binding, context))
                    m_matching.push_back(rule);
            if (m_matching.empty())
                return true;
            const auto candidate =
                tyr::planning::LabeledNode<Kind> { binding,
                                                   search.successor_generator->get_successor_node(tyr::planning::Node<Kind>(source.get_state(), 0),
                                                                                                  binding,
                                                                                                  *search.state_repository,
                                                                                                  *search.axiom_evaluator) };
            m_environment.get_dl_target_caches().clear(false);
            auto transition =
                m_environment.make_dl_transition_context(source.get_state(), candidate.node.get_state(), source.get_registers(), source.get_registers());
            std::optional<HistoriesView<Kind>> histories;
            for (const auto& [rule, variant] : m_matching)
            {
                if (stop())
                    return false;
                if (!xeffects_match(rule, source, binding, transition) || !effects_match(rule, transition))
                    continue;
                if (!histories)
                {
                    histories = update_histories(source, transition, binding, stop);
                    if (!histories)
                        return !stop();
                }
                auto target = source.get_data();
                target.state = candidate.node.get_state().get_index();
                target.memory_state = rule.get_target().get_index();
                target.histories = histories->get_index();
                auto step = Step(detail::ProgramOutcome::APPLIED, intern(target), m_task);
                step.rule = variant;
                step.planning_successor = candidate.pack();
                step.state_transition = datasets::StateGraphEdgeLabel { binding, ygg::float_t(1) };
                if (!emit(std::move(step)))
                    return false;
            }
            return true;
        };
        return search.successor_generator->for_each_applicable_action_binding(tyr::planning::Node<Kind>(source.get_state(), 0), action, std::ref(visit));
    }

public:
    using Step = detail::ProgramStep<Kind>;
    SuccessorExpander(TaskContextPtr<Kind> task, ProgramView program) :
        m_task(task ? std::move(task) : throw std::invalid_argument("ICP requires a task context.")),
        m_program(program),
        m_environment(*m_task, program)
    {
        if (&program.get_context() != m_task->domain_context->icp_repository.get())
            throw std::invalid_argument("ICP requires a program from the domain context repository.");
        const auto planning_task = m_task->search_context->task->get_task();
        auto empty = m_task->dl_builder.template get_builder<Denotation>(
            static_cast<ygg::uint_t>(planning_task.get_domain().get_constants().size() + planning_task.get_objects().size()));
        m_empty = runir::kr::dl::semantics::detail::materialize_denotation(empty, m_task->dl_builder, *m_task->dl_denotation_repository).first.get_index();
        const auto module = program.get_module();
        for (const auto transition : module.get_memory_transitions())
            for (const auto variant : transition)
                ygg::visit(
                    [&](auto rule)
                    {
                        m_rules[ygg::uint_t(rule.get_source().get_index())].push_back(variant);
                        if constexpr (std::same_as<decltype(rule), RuleView<CruleTag>>)
                        {
                            for (const auto action : m_task->search_context->task->get_task().get_domain().get_actions())
                                if (action.get_name().str() == rule.get_action_name())
                                {
                                    m_actions.emplace(ygg::uint_t(rule.get_index()), action);
                                    break;
                                }
                            if (!m_actions.contains(ygg::uint_t(rule.get_index())))
                                throw std::invalid_argument("Unknown ICP action schema.");
                        }
                    },
                    variant.get_variant());
        const auto features = module.template get_features<Concept>();
        m_reset_predecessors.resize(features.size());
        std::unordered_map<ygg::uint_t, size_t> slots;
        for (size_t i = 0; i < features.size(); ++i)
            slots.emplace(ygg::uint_t(features[i].get_index()), i);
        for (const auto& pair : module.get_reset_pairs())
            m_reset_predecessors.at(slots.at(ygg::uint_t(pair.after))).push_back(slots.at(ygg::uint_t(pair.before)));
    }

    const auto& get_task_context() const noexcept { return m_task; }
    ProgramStateView<Kind> initial_state(tyr::planning::StateView<Kind> state)
    {
        validate(state);
        const auto module = m_program.get_module();
        auto registers = checkout<Registers>(m_task->dl_builder);
        registers->concept_values.resize(module.template get_registers<Concept>().size());
        registers->role_values.resize(module.template get_registers<runir::kr::dl::RoleTag>().size());
        const auto stored_registers = get_or_create(*m_task->dl_denotation_repository, *registers).first;
        auto histories = ygg::Data<Histories> {};
        histories.concepts.resize(module.template get_features<Concept>().size(), m_empty);
        auto data = ygg::Data<ProgramState<Kind>> {};
        data.program = m_program.get_index();
        data.state = state.get_index();
        data.memory_state = module.get_entry_memory_state().get_index();
        data.registers = stored_registers.get_index();
        data.histories = intern(histories).get_index();
        return intern(data);
    }

    /// Natural rule order for greedy execution; grouped action enumeration for universal search.
    /// Callbacks must not reenter this expander or the planning successor generator.
    template<typename Emit, typename Stop>
    bool for_each_successor(ProgramStateView<Kind> state, ProgramSearchStatistics& statistics, Emit&& emit, Stop&& stop, bool grouped = false)
    {
        validate(state);
        if (stop())
            return false;
        m_environment.get_dl_caches().clear(false);
        auto context = m_environment.make_dl_context(state);
        bool emitted = false;
        const auto output = [&](Step step)
        {
            emitted = true;
            statistics.num_generated += step.status == detail::ProgramOutcome::APPLIED;
            return emit(std::move(step));
        };
        m_enabled.clear();
        const auto found = m_rules.find(ygg::uint_t(state.get_memory_state().get_index()));
        if (found != m_rules.end())
            for (const auto variant : found->second)
            {
                if (stop())
                    return false;
                const auto exhausted = ygg::visit(
                    [&](auto rule)
                    {
                        if (!conditions_are_compatible(rule, context))
                            return true;
                        if constexpr (std::same_as<decltype(rule), RuleView<CruleTag>>)
                        {
                            const auto action = m_actions.at(ygg::uint_t(rule.get_index()));
                            if (!grouped)
                                return emit_crules(action, { { rule, variant } }, state, output, stop);
                            auto found_group = std::find_if(m_enabled.begin(), m_enabled.end(), [&](const auto& entry) { return entry.action == action; });
                            if (found_group == m_enabled.end())
                                m_enabled.push_back({ action, { { rule, variant } } });
                            else
                                found_group->rules.emplace_back(rule, variant);
                            return true;
                        }
                        else
                            return emit_load(rule, variant, state, output, stop);
                    },
                    variant.get_variant());
                if (!exhausted)
                    return false;
            }
        for (const auto& group : m_enabled)
            if (!emit_crules(group.action, group.rules, state, output, stop))
                return false;
        if (stop())
            return false;
        return emitted || output(Step(detail::ProgramOutcome::NO_APPLICABLE_ACTION, state, m_task));
    }
};

}

#endif
