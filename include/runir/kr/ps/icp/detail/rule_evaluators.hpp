#ifndef RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATORS_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATORS_HPP_

#include "runir/kr/ps/icp/detail/rule_evaluation/crule.hpp"
#include "runir/kr/ps/icp/detail/rule_evaluation/load.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <algorithm>
#include <functional>
#include <span>
#include <variant>
#include <vector>

namespace runir::kr::ps::icp::detail
{

/// Prepared rule occurrences in module order, with one shared evaluation workspace.
template<tyr::TaskKind Kind>
class RuleEvaluators
{
    using CruleEvaluator = RuleEvaluator<Kind, CruleTag>;
    using Evaluator =
        std::variant<RuleEvaluator<Kind, LoadTag<runir::kr::dl::ConceptTag>>, RuleEvaluator<Kind, LoadTag<runir::kr::dl::RoleTag>>, CruleEvaluator>;
    using Action = tyr::formalism::planning::ActionView<tyr::LiftedTag>;
    struct ActionRules
    {
        Action action;
        std::vector<CruleEvaluator> rules;
    };

    RuleEvaluationWorkspace<Kind> m_workspace;
    std::vector<Evaluator> m_evaluators;
    // Groups retain their inner capacities even when fewer actions are enabled.
    std::vector<ActionRules> m_enabled;
    std::vector<CruleEvaluator> m_matching;

    template<EmitConcept<ProgramStep<Kind>> Emit, StopConcept Stop>
    bool emit_crules(runir::kr::ps::RuleEvaluationContext<IcpFamilyTag, Kind>& context,
                     Action action,
                     std::span<const CruleEvaluator> rules,
                     ProgramStateView<Kind> source,
                     Emit&& emit,
                     Stop&& stop)
    {
        auto& search = *get_task_context()->search_context;
        auto& environment = m_workspace.get_environment();
        auto source_context = context.make_dl_context(source);
        const auto visit = [&](tyr::planning::BorrowedActionBindingView<Kind> binding)
        {
            if (stop())
                return false;
            m_matching.clear();
            for (const auto& evaluator : rules)
                if (evaluator.xconditions_match(source_context, source, binding))
                    m_matching.push_back(evaluator);
            if (m_matching.empty())
                return true;
            const auto node = search.successor_generator->get_successor_node(tyr::planning::Node<Kind>(context.planning_state, 0),
                                                                             binding,
                                                                             *search.state_repository,
                                                                             *search.axiom_evaluator);
            const auto candidate =
                tyr::planning::LabeledNode<Kind, tyr::planning::StateView<Kind>, tyr::planning::BorrowedActionBindingView<Kind>> { binding, node };
            environment.reset_target();
            auto transition = context.make_dl_transition_context(source, candidate);
            auto histories = std::optional<HistoriesView<Kind>> {};
            auto label = std::optional<tyr::formalism::planning::ActionBindingView> {};
            for (const auto& evaluator : m_matching)
            {
                if (stop())
                    return false;
                if (!evaluator.matches(context, source, candidate))
                    continue;
                if (!histories)
                {
                    histories = m_workspace.update_histories(source, transition, binding, stop);
                    if (!histories)
                        return !stop();
                }
                if (stop())
                    return false;
                if (!label)
                    label = search.successor_generator->materialize_action_binding(binding);
                const auto labeled = tyr::planning::LabeledNode<Kind> { *label, candidate.node };
                if (!emit(evaluator.apply(m_workspace, source, labeled, *histories)))
                    return false;
            }
            return true;
        };
        return search.successor_generator->for_each_borrowed_applicable_action_binding(tyr::planning::Node<Kind>(context.planning_state, 0),
                                                                                       action,
                                                                                       std::ref(visit));
    }

public:
    RuleEvaluators(TaskContextPtr<Kind> task, ProgramView program) : m_workspace(std::move(task), program)
    {
        for (const auto transition : program.get_module().get_memory_transitions())
            for (const auto variant : transition)
                m_evaluators.push_back(ygg::visit([&]<RuleKind Tag>(RuleView<Tag> rule) -> Evaluator
                                                  { return RuleEvaluator<Kind, Tag>(*get_task_context(), rule, variant); },
                                                  variant.get_variant()));
        m_matching.reserve(m_evaluators.size());
        m_enabled.reserve(m_evaluators.size());
    }

    const auto& get_task_context() const noexcept { return m_workspace.get_task_context(); }
    auto get_program() const noexcept { return m_workspace.get_program(); }
    auto initial_state(tyr::planning::StateView<Kind> state) { return m_workspace.initial_state(state); }

    /// Callbacks borrow shared scratch and must not reenter the evaluators or planning generator.
    template<EmitConcept<ProgramStep<Kind>> Emit, StopConcept Stop>
    bool for_each_successor(ProgramStateView<Kind> state, Emit&& emit, Stop&& stop, bool grouped)
    {
        if (stop())
            return false;
        auto& environment = m_workspace.get_environment();
        environment.reset_source();
        auto source_context = environment.make_dl_context(state);
        auto context = runir::kr::ps::RuleEvaluationContext<IcpFamilyTag, Kind> { m_workspace, source_context.get_state() };
        bool emitted = false;
        const auto output = [&](ProgramStep<Kind> step)
        {
            emitted = true;
            return emit(std::move(step));
        };
        std::size_t num_enabled = 0;
        const auto memory = state.get_memory_state();
        for (const auto& rule : m_evaluators)
        {
            const auto exhausted = std::visit(
                [&]<typename Concrete>(const Concrete& evaluator)
                {
                    if (evaluator.get_rule().get_source() != memory)
                        return true;
                    if (stop())
                        return false;
                    if constexpr (std::same_as<Concrete, CruleEvaluator>)
                    {
                        if (!evaluator.is_applicable(source_context))
                            return true;
                        const auto action = evaluator.get_action();
                        if (!grouped)
                            return emit_crules(context, action, std::span<const CruleEvaluator>(&evaluator, 1), state, output, stop);
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
                        group->rules.push_back(evaluator);
                        return true;
                    }
                    else
                        return evaluator.emit(context, state, output, stop);
                },
                rule);
            if (!exhausted)
                return false;
        }
        for (std::size_t i = 0; i < num_enabled; ++i)
            if (!emit_crules(context, m_enabled[i].action, m_enabled[i].rules, state, output, stop))
                return false;
        if (stop())
            return false;
        return emitted || output(ProgramStep<Kind>(ProgramOutcome::NO_APPLICABLE_ACTION, state, get_task_context()));
    }
};

}  // namespace runir::kr::ps::icp::detail

#endif
