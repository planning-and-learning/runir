#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_DO_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_DO_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"

#include <algorithm>
#include <functional>
#include <optional>
#include <tyr/formalism/planning/action_view.hpp>
#include <tyr/planning/node.hpp>
#include <vector>

namespace runir::kr::ps::ext::detail
{

struct DoRuleWorkspace
{
    std::vector<runir::kr::dl::semantics::ConceptDenotationView> denotations;
};

template<tyr::TaskKind Kind>
class DoRuleEvaluator
{
    RuleView<DoTag> m_rule;
    RuleVariantView m_variant;
    std::optional<tyr::formalism::planning::ActionView<tyr::LiftedTag>> m_action;

public:
    using RuleTag = DoTag;
    DoRuleEvaluator(RuleView<DoTag> rule, RuleVariantView variant, const tyr::planning::Task<Kind>& task) : m_rule(rule), m_variant(variant)
    {
        for (const auto action : task.get_task().get_domain().get_actions())
            if (action.get_name().str() == rule.get_action_name())
            {
                m_action = action;
                break;
            }
    }
    auto rule() const noexcept { return m_rule; }
    auto variant() const noexcept { return m_variant; }

private:
    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    auto& evaluate_do_arguments(Context& context, DoRuleWorkspace& workspace, RuleView<DoTag> rule, S state, const PS& planning_state)
    {
        const auto arguments = rule.get_action_arguments();
        auto& denotations = workspace.denotations;
        denotations.clear();
        auto state_context =
            context.environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        for (auto argument : arguments)
            denotations.push_back(evaluate(argument, state_context));
        return denotations;
    }

    template<ygg::formalism::RelationBindingViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>, tyr::formalism::ObjectTag> Binding,
             typename ConceptDenotations>
    static bool action_matches_do_arguments(RuleView<DoTag> rule, Binding action, const ConceptDenotations& denotations)
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

    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool do_effects_match(Context& context, RuleView<DoTag> rule, S state, const PS& planning_state, const PS& target_state)
    {
        auto transition = context.environment.make_dl_transition_context(planning_state,
                                                                         target_state,
                                                                         state.get_module_state().get_arguments(),
                                                                         state.get_module_state().get_registers(),
                                                                         state.get_module_state().get_registers());
        return is_compatible_with(rule, transition);
    }

    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool do_rule_matches(Context& context,
                         DoRuleWorkspace& workspace,
                         RuleView<DoTag> rule,
                         S state,
                         const PS& planning_state,
                         tyr::formalism::planning::ActionBindingView action,
                         const PS& target_state)
    {
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment))
            return false;
        const auto& denotations = evaluate_do_arguments(context, workspace, rule, state, planning_state);
        return action_matches_do_arguments(rule, action, denotations) && do_effects_match(context, rule, state, planning_state, target_state);
    }

public:
    template<typename Context, typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit(Context& context, S state, const PS& planning_state, Emit&& emit, Stop&& stop, DoRuleWorkspace& workspace)
    {
        const auto rule = m_rule;
        const auto rule_variant = m_variant;
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment))
            return true;
        const auto& denotations = evaluate_do_arguments(context, workspace, rule, state, planning_state);
        if (std::ranges::any_of(denotations, [](const auto& denotation) { return denotation.get().count() == 0; }))
            return true;
        auto& search = *context.task_context->search_context;
        if (m_action)
        {
            const auto visit = [&](tyr::planning::BorrowedActionBindingView<Kind> binding)
            {
                if (stop())
                    return false;
                if (!action_matches_do_arguments(rule, binding, denotations))
                    return true;
                const auto candidate = context.storage.successor(planning_state, binding);
                context.environment.reset_target();
                if (!do_effects_match(context, rule, state, planning_state, candidate.get_state()))
                    return true;
                if (stop())
                    return false;
                const auto labeled = tyr::planning::LabeledNode<PS> { search.successor_generator->materialize_action_binding(binding), candidate };
                return emit(detail::planning_step(context.storage, state, labeled, rule_variant, rule.get_target(), context.task_context));
            };
            return search.successor_generator->for_each_borrowed_applicable_action_binding(tyr::planning::Node<PS>(planning_state, 0),
                                                                                           *m_action,
                                                                                           std::ref(visit));
        }
        return true;
    }
    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool matches(Context& context, S state, const PS& planning_state, const tyr::planning::LabeledNode<PS>& candidate, DoRuleWorkspace& workspace)
    {
        return do_rule_matches(context, workspace, m_rule, state, planning_state, candidate.label, candidate.node.get_state());
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
