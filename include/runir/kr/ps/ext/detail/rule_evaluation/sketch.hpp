#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_SKETCH_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_SKETCH_HPP_

#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"

#include <tyr/planning/node.hpp>
#include <utility>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
class SketchRuleEvaluator
{
    RuleView<SketchTag> m_rule;
    RuleVariantView m_variant;

public:
    using RuleTag = SketchTag;
    SketchRuleEvaluator(RuleView<SketchTag> rule, RuleVariantView variant) : m_rule(rule), m_variant(variant) {}
    auto rule() const noexcept { return m_rule; }
    auto variant() const noexcept { return m_variant; }

private:
    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool sketch_rule_matches_state(Context& context, RuleView<SketchTag> rule, S state, const PS& planning_state, const PS& target_state)
    {
        if (!ext::has_current_source(rule, state))
            return false;
        auto transition = context.environment.make_dl_transition_context(planning_state,
                                                                         target_state,
                                                                         state.get_module_state().get_arguments(),
                                                                         state.get_module_state().get_registers(),
                                                                         state.get_module_state().get_registers());
        return is_compatible_with(rule, transition);
    }

public:
    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool matches(Context& context, S state, const PS& planning_state, const tyr::planning::LabeledNode<PS>& candidate)
    {
        return !m_rule.get_effects().empty() && sketch_rule_matches_state(context, m_rule, state, planning_state, candidate.node.get_state());
    }

    template<typename Context, ProgramStateViewConcept<Kind> S>
    auto control_step(Context& context, S state)
    {
        auto target = context.storage.module_();
        *target = state.get_module_state().get_data();
        target->memory_state = m_rule.get_target().get_index();
        return detail::applied(context.storage.store(std::move(target), state.get_call_stack()), m_variant, context.task_context);
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
