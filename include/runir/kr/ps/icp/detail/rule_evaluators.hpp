#ifndef RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATORS_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATORS_HPP_

#include "runir/kr/ps/icp/detail/rule_evaluation/crule.hpp"
#include "runir/kr/ps/icp/detail/rule_evaluation/load.hpp"

#include <algorithm>
#include <functional>
#include <span>
#include <variant>

namespace runir::kr::ps::icp::detail
{

/// Frozen rule records and occurrence schedules, with one shared evaluation workspace.
template<tyr::TaskKind Kind>
class RuleEvaluators
{
    using CruleEvaluator = RuleEvaluator<Kind, CruleTag>;
    using Evaluator =
        std::variant<RuleEvaluator<Kind, LoadTag<runir::kr::dl::ConceptTag>>, RuleEvaluator<Kind, LoadTag<runir::kr::dl::RoleTag>>, CruleEvaluator>;
    using Action = tyr::formalism::planning::ActionView<tyr::LiftedTag>;
    struct Schedule
    {
        ygg::uint_t memory;
        std::size_t begin;
        std::size_t end;
    };
    struct ActionRules
    {
        Action action;
        std::vector<std::size_t> rules;
    };

    RuleEvaluationWorkspace<Kind> m_workspace;
    std::vector<Evaluator> m_evaluators;
    std::vector<std::size_t> m_schedule;
    std::vector<Schedule> m_sources;
    // Slots retain their inner capacities even when fewer action groups are enabled.
    std::vector<ActionRules> m_enabled;
    std::vector<std::size_t> m_matching;

    template<typename Emit, typename Stop>
    bool emit_crules(Action action, std::span<const std::size_t> rules, ProgramStateView<Kind> source, Emit&& emit, Stop&& stop)
    {
        auto& search = *get_task_context()->search_context;
        auto& environment = m_workspace.get_environment();
        auto context = environment.make_dl_context(source);
        const auto visit = [&](tyr::formalism::planning::ActionBindingView binding)
        {
            if (stop())
                return false;
            m_matching.clear();
            for (const auto slot : rules)
                if (std::get<CruleEvaluator>(m_evaluators[slot]).xconditions_match(source, binding, context))
                    m_matching.push_back(slot);
            if (m_matching.empty())
                return true;
            const auto candidate = tyr::planning::LabeledNode<tyr::planning::StateView<Kind>> {
                binding,
                search.successor_generator->get_successor_node(tyr::planning::Node<tyr::planning::StateView<Kind>>(source.get_state(), 0),
                                                               binding,
                                                               *search.state_repository,
                                                               *search.axiom_evaluator)
            };
            environment.reset_target();
            auto transition =
                environment.make_dl_transition_context(source.get_state(), candidate.node.get_state(), source.get_registers(), source.get_registers());
            auto histories = std::optional<HistoriesView<Kind>> {};
            for (const auto slot : m_matching)
            {
                if (stop())
                    return false;
                const auto& evaluator = std::get<CruleEvaluator>(m_evaluators[slot]);
                if (!evaluator.transition_matches(source, binding, transition))
                    continue;
                if (!histories)
                {
                    histories = m_workspace.update_histories(source, transition, binding, stop);
                    if (!histories)
                        return !stop();
                }
                if (!emit(evaluator.apply(source, candidate, *histories, m_workspace)))
                    return false;
            }
            return true;
        };
        return search.successor_generator->for_each_applicable_action_binding(tyr::planning::Node<tyr::planning::StateView<Kind>>(source.get_state(), 0),
                                                                              action,
                                                                              std::ref(visit));
    }

public:
    RuleEvaluators(TaskContextPtr<Kind> task, ProgramView program) : m_workspace(std::move(task), program)
    {
        auto slots = std::unordered_map<ygg::uint_t, std::size_t> {};
        auto occurrences = std::vector<std::pair<ygg::uint_t, std::size_t>> {};
        for (const auto transition : program.get_module().get_memory_transitions())
            for (const auto variant : transition)
            {
                const auto [slot, inserted] = slots.try_emplace(ygg::uint_t(variant.get_index()), m_evaluators.size());
                if (inserted)
                    m_evaluators.push_back(ygg::visit([&]<RuleKind Tag>(RuleView<Tag> rule) -> Evaluator
                                                      { return RuleEvaluator<Kind, Tag>(*get_task_context(), rule, variant); },
                                                      variant.get_variant()));
                const auto memory =
                    std::visit([](const auto& evaluator) { return ygg::uint_t(evaluator.get_rule().get_source().get_index()); }, m_evaluators[slot->second]);
                occurrences.emplace_back(memory, slot->second);
            }
        std::stable_sort(occurrences.begin(), occurrences.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
        m_schedule.reserve(occurrences.size());
        for (const auto& [memory, slot] : occurrences)
        {
            if (m_sources.empty() || m_sources.back().memory != memory)
                m_sources.push_back({ memory, m_schedule.size(), m_schedule.size() });
            m_schedule.push_back(slot);
            m_sources.back().end = m_schedule.size();
        }
        m_matching.reserve(m_schedule.size());
        m_enabled.reserve(m_evaluators.size());
    }

    const auto& get_task_context() const noexcept { return m_workspace.get_task_context(); }
    auto get_program() const noexcept { return m_workspace.get_program(); }
    auto initial_state(tyr::planning::StateView<Kind> state) { return m_workspace.initial_state(state); }

    /// Callbacks borrow shared scratch and must not reenter the evaluators or planning generator.
    template<typename Emit, typename Stop>
    bool for_each_successor(ProgramStateView<Kind> state, Emit&& emit, Stop&& stop, bool grouped)
    {
        if (stop())
            return false;
        auto& environment = m_workspace.get_environment();
        environment.reset_source();
        auto context = environment.make_dl_context(state);
        bool emitted = false;
        const auto output = [&](ProgramStep<Kind> step)
        {
            emitted = true;
            return emit(std::move(step));
        };
        std::size_t num_enabled = 0;
        const auto memory = ygg::uint_t(state.get_memory_state().get_index());
        const auto found = std::lower_bound(m_sources.begin(), m_sources.end(), memory, [](const auto& entry, auto value) { return entry.memory < value; });
        if (found != m_sources.end() && found->memory == memory)
            for (auto position = found->begin; position != found->end; ++position)
            {
                if (stop())
                    return false;
                const auto slot = m_schedule[position];
                const auto exhausted = std::visit(
                    [&](const auto& evaluator)
                    {
                        if (!evaluator.applicable(context))
                            return true;
                        if constexpr (std::same_as<std::remove_cvref_t<decltype(evaluator)>, CruleEvaluator>)
                        {
                            const auto action = evaluator.get_action();
                            if (!grouped)
                                return emit_crules(action, std::span<const std::size_t>(&slot, 1), state, output, stop);
                            auto group =
                                std::find_if(m_enabled.begin(), m_enabled.begin() + num_enabled, [&](const auto& entry) { return entry.action == action; });
                            if (group == m_enabled.begin() + num_enabled)
                            {
                                if (num_enabled == m_enabled.size())
                                    m_enabled.push_back({ action, {} });
                                group = m_enabled.begin() + num_enabled++;
                                group->action = action;
                                group->rules.clear();
                            }
                            group->rules.push_back(slot);
                            return true;
                        }
                        else
                            return evaluator.emit(state, m_workspace, output, stop);
                    },
                    m_evaluators[slot]);
                if (!exhausted)
                    return false;
            }
        for (std::size_t i = 0; i < num_enabled; ++i)
            if (!emit_crules(m_enabled[i].action, m_enabled[i].rules, state, output, stop))
                return false;
        if (stop())
            return false;
        return emitted || output(ProgramStep<Kind>(ProgramOutcome::NO_APPLICABLE_ACTION, state, get_task_context()));
    }
};

}  // namespace runir::kr::ps::icp::detail

#endif
