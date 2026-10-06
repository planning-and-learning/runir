#ifndef RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_CRULE_HPP_
#define RUNIR_KR_PS_ICP_DETAIL_RULE_EVALUATION_CRULE_HPP_

#include "runir/kr/ps/icp/detail/rule_evaluation/context.hpp"

#include <cista/containers/variant.h>
#include <tyr/formalism/planning/action_view.hpp>

namespace runir::kr::ps::icp::detail
{

template<tyr::TaskKind Kind>
class RuleEvaluator<Kind, CruleTag>
{
    using Action = tyr::formalism::planning::ActionView<tyr::LiftedTag>;
    RuleView<CruleTag> m_rule;
    RuleVariantView m_variant;
    Action m_action;

    static Action resolve_action(TaskContext<Kind>& task, RuleView<CruleTag> rule)
    {
        for (const auto action : task.search_context->task->get_task().get_domain().get_actions())
            if (action.get_name().str() == rule.get_action_name())
                return action;
        throw std::invalid_argument("Unknown ICP action schema.");
    }

    template<ygg::formalism::RelationBindingViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>, tyr::formalism::ObjectTag> Binding>
    std::optional<ygg::uint_t>
    resolve(const ::cista::offset::variant<ArgumentPosition, ygg::Index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>>>& reference,
            runir::kr::dl::semantics::RegisterValuesView registers,
            Binding binding) const
    {
        return reference.apply(
            [&](auto ref) -> std::optional<ygg::uint_t>
            {
                if constexpr (std::same_as<decltype(ref), ArgumentPosition>)
                    return ygg::uint_t(binding.get_objects().at(ygg::uint_t(ref)).get_index());
                else
                {
                    const auto reg = ygg::make_view(ref, m_rule.get_context().get_dl_repository());
                    const auto object = registers.at(reg.get_identifier());
                    return object ? std::optional(ygg::uint_t(object.value().get_index())) : std::nullopt;
                }
            });
    }

public:
    RuleEvaluator(TaskContext<Kind>& task, RuleView<CruleTag> rule, RuleVariantView variant) :
        m_rule(rule),
        m_variant(variant),
        m_action(resolve_action(task, rule))
    {
    }

    auto get_rule() const noexcept { return m_rule; }
    /// The aggregate groups rules by their resolved action before generating candidates.
    auto get_action() const noexcept { return m_action; }
    /// Skip disabled rules before action enumeration; matches() also checks this for direct callers.
    template<runir::kr::dl::semantics::EvaluationContextConcept<ExtFamilyTag, Kind> Context>
    bool is_applicable(Context& context) const
    {
        return conditions_are_compatible<Kind>(m_rule, context);
    }

    /// Reject a binding before generating its target; matches() also checks this for direct callers.
    template<runir::kr::dl::semantics::EvaluationContextConcept<ExtFamilyTag, Kind> Context,
             ygg::formalism::RelationBindingViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>, tyr::formalism::ObjectTag> Binding>
    bool xconditions_match(Context& context, ProgramStateView<Kind> state, Binding binding) const
    {
        for (const auto condition : m_rule.get_xconditions())
        {
            const auto object = resolve(condition.get_object_reference(), state.get_registers(), binding);
            if (!object)
                return false;
            const auto contains = evaluate<Kind>(condition.get_concept_feature(), context).get().test(*object);
            if (contains != (condition.get_operation() == ConditionOperation::BELONGS))
                return false;
        }
        return true;
    }

    /// Match a supplied candidate independently of the aggregate's early binding filter.
    /// History admission remains shared by the aggregate across matching rules.
    template<runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy,
             ygg::formalism::RelationBindingViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>, tyr::formalism::ObjectTag> Binding>
    bool matches(runir::kr::ps::RuleEvaluationContext<IcpFamilyTag, Kind, void, tyr::planning::StateView<Kind>, EvaluationPolicy>& context,
                 ProgramStateView<Kind> state,
                 const tyr::planning::LabeledNode<Kind, tyr::planning::StateView<Kind>, Binding>& candidate) const
    {
        const auto memory = state.get_memory_state();
        const auto source = m_rule.get_source();
        const auto action = candidate.label.get_relation();
        if (&memory.get_context() != &source.get_context() || memory != source || &action.get_context() != &m_action.get_context() || action != m_action)
            return false;
        auto source_context = context.make_dl_context(state);
        if (!is_applicable(source_context) || !xconditions_match(source_context, state, candidate.label))
            return false;
        auto transition = context.make_dl_transition_context(state, candidate);
        for (const auto effect : m_rule.get_xeffects())
        {
            const auto object = resolve(effect.get_object_reference(), state.get_registers(), candidate.label);
            if (!object)
                return false;
            const auto before = evaluate<Kind>(effect.get_concept_feature(), transition.get_source_context()).get().test(*object);
            const auto after = evaluate<Kind>(effect.get_concept_feature(), transition.get_target_context()).get().test(*object);
            if (effect.get_operation() == EffectOperation::ENTER ? (before || !after) : (!before || after))
                return false;
        }
        return all_compatible<Kind>(m_rule.get_effects(), transition);
    }

    /// Apply a match after the aggregate admits shared histories and publishes the binding.
    template<runir::kr::dl::semantics::EvaluationPolicyConcept<ExtFamilyTag, Kind> EvaluationPolicy>
    ProgramStep<Kind> apply(RuleEvaluationWorkspace<Kind, EvaluationPolicy>& workspace,
                            ProgramStateView<Kind> source,
                            const tyr::planning::LabeledNode<Kind>& candidate,
                            HistoriesView<Kind> histories) const
    {
        auto target = source.get_data();
        target.state = candidate.node.get_state().get_index();
        target.memory_state = m_rule.get_target().get_index();
        target.histories = histories.get_index();
        auto step = ProgramStep<Kind>(ProgramOutcome::APPLIED, workspace.intern(target), workspace.get_task_context());
        step.rule = m_variant;
        step.planning_successor = candidate.pack();
        step.state_transition = datasets::StateGraphEdgeLabel { candidate.label, ygg::float_t(1) };
        return step;
    }
};

}  // namespace runir::kr::ps::icp::detail

#endif
