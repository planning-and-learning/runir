#ifndef RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_CRULE_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_CRULE_HPP_

#include "runir/datasets/successors.hpp"
#include "runir/kr/ps/icp/detail/rule_evaluation/context.hpp"

#include <tyr/formalism/planning/action_view.hpp>
#include <urd/elements/flat.hpp>
#include <variant>

namespace runir::kr::ps::icp::detail
{

template<tyr::TaskKind Kind>
class RuleEvaluator<Kind, CruleTag>
{
    using Action = tyr::formalism::ActionView<tyr::LiftedTag>;
    RuleView<CruleTag> m_rule;
    RuleVariantView m_variant;
    Action m_action;

    static Action resolve_action(TaskContext<Kind>& task, RuleView<CruleTag> rule)
    {
        for (const auto action : task.search_context->task->get_task().domain().actions())
            if (action.name().str() == rule.action_name())
                return action;
        throw std::invalid_argument("Unknown ICP action schema.");
    }

    template<ygg::RowViewConcept<tyr::formalism::Action<tyr::LiftedTag>> Binding>
    [[nodiscard]] std::optional<ygg::uint_t> resolve(const auto& reference, runir::kr::dl::semantics::RegisterValuesView registers, Binding binding) const
    {
        return std::visit(
            [&]<typename Ref>(const Ref& ref) -> std::optional<ygg::uint_t>
            {
                if constexpr (std::same_as<Ref, ArgumentPosition>)
                    return static_cast<ygg::uint_t>(binding.value(ref.value));  // an object column: the object's index
                else
                {
                    const auto object = registers.at(ref.identifier());
                    return object ? std::optional(ygg::uint_t(object.value().index().value)) : std::nullopt;
                }
            },
            reference);
    }

public:
    RuleEvaluator(TaskContext<Kind>& task, RuleView<CruleTag> rule, RuleVariantView variant) :
        m_rule(rule),
        m_variant(variant),
        m_action(resolve_action(task, rule))
    {
    }

    [[nodiscard]] auto get_rule() const noexcept { return m_rule; }

    /// The aggregate groups rules by their resolved action before generating candidates.
    [[nodiscard]] auto get_action() const noexcept { return m_action; }

    /// Skip disabled rules before action enumeration; matches() also checks this for direct callers.
    template<runir::kr::dl::semantics::EvaluationContextConcept<Kind> Context>
    bool is_applicable(Context& context) const
    { return all_compatible<Kind, IcpFamilyTag>(m_rule.conditions(), context); }

    /// Reject a binding before generating its target; matches() also checks this for direct callers.
    template<runir::kr::dl::semantics::EvaluationContextConcept<Kind> Context, ygg::RowViewConcept<tyr::formalism::Action<tyr::LiftedTag>> Binding>
    bool xconditions_match(Context& context, ProgramStateView<Kind> state, Binding binding) const
    {
        for (const auto condition : m_rule.xconditions())
        {
            const auto object = resolve(condition.object(), state.registers(), binding);
            if (!object)
                return false;
            const auto contains = evaluate<Kind>(condition.feature(), context).value().test(*object);
            if (contains != (condition.operation() == ConditionOperation::BELONGS))
                return false;
        }
        return true;
    }

    /// Match a supplied candidate independently of the aggregate's early binding filter.
    /// History admission remains shared by the aggregate across matching rules.
    template<runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy, ygg::RowViewConcept<tyr::formalism::Action<tyr::LiftedTag>> Binding>
    bool matches(runir::kr::ps::RuleEvaluationContext<IcpFamilyTag, Kind, void, tyr::planning::SharedStateBuilderView<Kind>, EvaluationPolicy>& context,
                 ProgramStateView<Kind> state,
                 const runir::datasets::LabeledPlanningNode<Kind, tyr::planning::SharedStateBuilderView<Kind>, Binding>& candidate) const
    {
        const auto memory = state.memory_state();
        const auto source = m_rule.source();
        // The candidate's action, as its binding holds it: this rule's only where it is that interned action.
        if (&memory.context() != &source.context() || memory != source || !urd::is_interned_as(candidate.label().relation(), m_action))
            return false;
        auto source_context = context.make_dl_context(state);
        if (!is_applicable(source_context) || !xconditions_match(source_context, state, candidate.label()))
            return false;
        auto transition = context.make_dl_transition_context(state, candidate);
        for (const auto effect : m_rule.xeffects())
        {
            const auto object = resolve(effect.object(), state.registers(), candidate.label());
            if (!object)
                return false;
            const auto before = evaluate<Kind>(effect.feature(), transition.get_source_context()).value().test(*object);
            const auto after = evaluate<Kind>(effect.feature(), transition.get_target_context()).value().test(*object);
            if (effect.operation() == EffectOperation::ENTER ? (before || !after) : (!before || after))
                return false;
        }
        return all_compatible<Kind, IcpFamilyTag>(m_rule.effects(), transition);
    }

    /// Apply a match after the aggregate admits shared histories and publishes the binding.
    template<runir::kr::dl::semantics::EvaluationPolicyConcept<Kind> EvaluationPolicy>
    ProgramStep<IcpFamilyTag, Kind> apply(RuleEvaluationWorkspace<Kind, EvaluationPolicy>& workspace,
                                          ProgramStateView<Kind> source,
                                          const runir::datasets::LabeledPlanningNode<Kind>& candidate,
                                          HistoriesView histories) const
    {
        auto target = urd::data<ProgramState<Kind>>(source.program().index(),
                                                    m_rule.target().index(),
                                                    source.registers().index(),
                                                    histories.index(),
                                                    candidate.node().state().pack().index());
        auto step = ProgramStep<IcpFamilyTag, Kind>(ProgramOutcome::APPLIED, workspace.intern(target), workspace.get_task_context());
        step.rule = m_variant;
        step.planning_successor = candidate.pack();
        step.state_transition = urd::data<datasets::StateGraphEdgeLabel> { candidate.label(), static_cast<ygg::float_t>(1) };
        return step;
    }
};

}  // namespace runir::kr::ps::icp::detail

#endif
