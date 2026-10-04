#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_ACTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_ACTION_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/action_rule_contract_error.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"

#include <concepts>
#include <span>
#include <sstream>
#include <string_view>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <tyr/planning/state_view.hpp>
#include <tyr/planning/successor_generator.hpp>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::StateViewConcept State>
[[noreturn]] void action_rule_contract_error(RuleView<ActionTag> rule, const State& state, std::span<const ygg::uint_t> tuple, std::string_view reason)
{
    auto message = std::ostringstream {};
    message << "Action rule " << ygg::uint_t(rule.get_index()) << " for '" << rule.get_action_name().str() << "' in state";
    if constexpr (requires { state.get_index(); })
        message << ' ' << ygg::uint_t(state.get_index());
    message << " with tuple (";
    for (size_t i = 0; i < tuple.size(); ++i)
        message << (i ? ", " : "") << tuple[i];
    message << "): " << reason;
    throw ActionRuleContractError(message.str());
}

struct ActionRuleWorkspace
{
    ygg::Data<tyr::formalism::RelationBinding<tyr::formalism::planning::Action<tyr::LiftedTag>>> binding;
    std::vector<tyr::formalism::planning::ObjectView> objects;
    std::vector<ygg::uint_t> tuple;
};

template<tyr::TaskKind Kind>
class ActionRuleEvaluator
{
    RuleView<ActionTag> m_rule;
    RuleVariantView m_variant;
    tyr::formalism::planning::ActionView<tyr::LiftedTag> m_action;

    template<tyr::planning::StateViewConcept<Kind> State>
    void validate_query(const State& state, size_t arity) const
    {
        // Query contracts are checked only when the rule is evaluated.
        if (m_action.get_arity() != arity)
            action_rule_contract_error(m_rule, state, {}, "query arity does not match action arity");
    }

    template<tyr::planning::StateViewConcept<Kind> State>
    tyr::formalism::planning::ActionBindingView applicable_binding(tyr::planning::SuccessorGenerator<Kind>& generator,
                                                                   const State& planning_state,
                                                                   std::span<const ygg::uint_t> tuple,
                                                                   ActionRuleWorkspace& workspace) const
    {
        auto& binding = workspace.binding;
        binding.relation = m_action.get_index();
        binding.objects.clear();
        workspace.objects.clear();
        for (const auto object : tuple)
        {
            const auto index = ygg::Index<tyr::formalism::Object>(object);
            binding.objects.push_back(index);
            workspace.objects.push_back(ygg::make_view(index, *generator.get_task()->get_repository()));
        }
        // Policy applicability has no carried path metric.
        const auto status = generator.check_action_binding(tyr::planning::Node<State>(planning_state, 0), m_action, workspace.objects);
        if (status == tyr::planning::ActionBindingStatus::OUTSIDE_PARAMETER_DOMAIN)
            action_rule_contract_error(m_rule, planning_state, tuple, "object is outside the action parameter domain");
        if (status != tyr::planning::ActionBindingStatus::APPLICABLE)
            action_rule_contract_error(m_rule, planning_state, tuple, "offered action is not applicable");
        return tyr::formalism::planning::insert(*generator.get_task()->get_repository(), binding).first;
    }

public:
    using RuleTag = ActionTag;
    ActionRuleEvaluator(RuleView<ActionTag> rule, RuleVariantView variant, tyr::formalism::planning::ActionView<tyr::LiftedTag> action) :
        m_rule(rule),
        m_variant(variant),
        m_action(action)
    {
    }
    auto rule() const noexcept { return m_rule; }
    auto variant() const noexcept { return m_variant; }

private:
    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    void check_action_effects(Context& context,
                              RuleView<ActionTag> rule,
                              S state,
                              const PS& planning_state,
                              const tyr::planning::LabeledNode<PS>& candidate,
                              std::span<const ygg::uint_t> tuple)
    {
        context.environment.reset_target();
        auto transition = context.environment.make_dl_transition_context(planning_state,
                                                                         candidate.node.get_state(),
                                                                         state.get_module_state().get_arguments(),
                                                                         state.get_module_state().get_registers(),
                                                                         state.get_module_state().get_registers());
        if (!all_compatible(rule.get_effects(), transition))
            detail::action_rule_contract_error(rule, planning_state, tuple, "offered transition violates declared effects");
    }

public:
    template<typename Context, typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit(Context& context, S state, const PS& planning_state, Emit&& emit, Stop&& stop, ActionRuleWorkspace& workspace)
    {
        const auto rule = m_rule;
        const auto rule_variant = m_variant;
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment))
            return true;
        auto state_context =
            context.environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        const auto query = evaluate(rule.get_query_feature(), state_context);
        validate_query(planning_state, query.arity());
        for (std::size_t i = 0; i < query.size(); ++i)
        {
            if (stop())
                return false;
            const auto tuple = query[i];
            const auto binding = applicable_binding(*context.task_context->search_context->successor_generator, planning_state, tuple, workspace);
            const auto candidate = context.storage.successor(planning_state, binding);
            check_action_effects(context, rule, state, planning_state, candidate, tuple);
            if (!emit(detail::planning_step(context.storage, state, candidate, rule_variant, rule.get_target(), context.task_context)))
                return false;
        }
        return true;
    }
    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool matches(Context& context, S state, const PS& planning_state, const tyr::planning::LabeledNode<PS>& candidate, ActionRuleWorkspace& workspace)
    {
        const auto rule = m_rule;
        const auto action = candidate.label.get_relation();
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment) || &action.get_context() != &m_action.get_context()
            || action != m_action)
            return false;
        auto state_context =
            context.environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        const auto query = evaluate(rule.get_query_feature(), state_context);
        validate_query(planning_state, query.arity());
        workspace.tuple.clear();
        for (const auto object : candidate.label.get_objects())
            workspace.tuple.push_back(ygg::uint_t(object.get_index()));
        if (workspace.tuple.size() != query.arity() || !query.contains(std::span<const ygg::uint_t>(workspace.tuple)))
            return false;
        applicable_binding(*context.task_context->search_context->successor_generator, planning_state, workspace.tuple, workspace);
        check_action_effects(context, rule, state, planning_state, candidate, workspace.tuple);
        return true;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
