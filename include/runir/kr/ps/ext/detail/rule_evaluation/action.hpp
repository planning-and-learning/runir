#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_ACTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_ACTION_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/action_rule_contract_error.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/detail/execution_step.hpp"
#include "runir/kr/ps/ext/detail/rule_evaluation/context.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"
#include "runir/kr/ps/rule_evaluator_concepts.hpp"

#include <concepts>
#include <span>
#include <sstream>
#include <string_view>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <tyr/planning/node.hpp>
#include <tyr/planning/state_view.hpp>
#include <tyr/planning/successor_generator.hpp>
#include <utility>
#include <yggdrasil/core/concepts.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::planning::StateViewConcept State, ygg::SizedForwardRangeOf<ygg::Index<tyr::formalism::Object>> Row>
[[noreturn]] void action_rule_contract_error(RuleView<ActionTag> rule, const State& state, const Row& tuple, std::string_view reason)
{
    auto message = std::ostringstream {};
    message << "Action rule " << ygg::uint_t(rule.get_index()) << " for '" << rule.get_action_name().str() << "' in state";
    if constexpr (requires { state.get_index(); })
        message << ' ' << ygg::uint_t(state.get_index());
    message << " with tuple (";
    auto separator = "";
    for (const auto object : tuple)
    {
        message << separator << ygg::uint_t(object);
        separator = ", ";
    }
    message << "): " << reason;
    throw ActionRuleContractError(message.str());
}

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
            action_rule_contract_error(m_rule, state, std::span<const ygg::Index<tyr::formalism::Object>> {}, "query arity does not match action arity");
    }

    template<tyr::planning::StateViewConcept<Kind> State, ygg::SizedForwardRangeOf<ygg::Index<tyr::formalism::Object>> Row>
    void require_applicable(tyr::planning::ActionBindingStatus status, const State& planning_state, const Row& objects) const
    {
        if (status == tyr::planning::ActionBindingStatus::OUTSIDE_PARAMETER_DOMAIN)
            action_rule_contract_error(m_rule, planning_state, objects, "object is outside the action parameter domain");
        if (status != tyr::planning::ActionBindingStatus::APPLICABLE)
            action_rule_contract_error(m_rule, planning_state, objects, "offered action is not applicable");
    }

public:
    using RuleTag = ActionTag;
    ActionRuleEvaluator(RuleView<ActionTag> rule, RuleVariantView variant, tyr::formalism::planning::ActionView<tyr::LiftedTag> action) :
        m_rule(rule),
        m_variant(variant),
        m_action(action)
    {
    }
    auto get_rule() const noexcept { return m_rule; }
    auto get_variant() const noexcept { return m_variant; }

private:
    template<ExecutionStorageConcept<Kind> Storage,
             ProgramStateViewConcept<Kind> State,
             tyr::planning::StateViewConcept<Kind> PlanningState,
             ygg::SizedForwardRangeOf<ygg::Index<tyr::formalism::Object>> Row>
    void check_action_effects(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context,
                              RuleView<ActionTag> rule,
                              State state,
                              const PlanningState& planning_state,
                              const tyr::planning::Node<Kind, PlanningState>& candidate,
                              const Row& tuple) const
    {
        context.environment.reset_target();
        auto transition = context.environment.make_dl_transition_context(planning_state,
                                                                         candidate.get_state(),
                                                                         state.get_module_state().get_arguments(),
                                                                         state.get_module_state().get_registers(),
                                                                         state.get_module_state().get_registers());
        if (!all_compatible<Kind>(rule.get_effects(), transition))
            detail::action_rule_contract_error(rule, planning_state, tuple, "offered transition violates declared effects");
    }

public:
    template<ExecutionStorageConcept<Kind> Storage,
             EmitConcept<ProgramStep<Kind, Storage>> Emit,
             StopConcept Stop,
             ExecutionStateViewConcept<Storage> State,
             tyr::planning::StateViewConcept<Kind> PlanningState>
    bool emit(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context, State state, Emit&& emit, Stop&& stop) const
    {
        const auto& planning_state = context.planning_state;
        const auto rule = m_rule;
        const auto rule_variant = m_variant;
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment))
            return true;
        auto state_context = context.make_dl_context(state);
        const auto query = evaluate<Kind>(rule.get_query_feature(), state_context);
        validate_query(planning_state, query.arity());
        for (const auto objects : query)
        {
            if (stop())
                return false;
            // Policy applicability has no carried path metric.
            const auto offered =
                context.task_context->search_context->successor_generator->try_get_applicable_action_binding(tyr::planning::Node(planning_state, 0),
                                                                                                             m_action,
                                                                                                             objects);
            require_applicable(offered.status, planning_state, objects.get_data());
            const auto candidate = context.storage.successor(planning_state, *offered.binding);
            check_action_effects(context, rule, state, planning_state, candidate, objects.get_data());
            const auto labeled = tyr::planning::LabeledNode { *offered.binding, candidate };
            if (!emit(detail::planning_step(context.storage, state, labeled, rule_variant, rule.get_target(), context.task_context)))
                return false;
        }
        return true;
    }
    template<ExecutionStorageConcept<Kind> Storage,
             ProgramStateViewConcept<Kind> State,
             tyr::planning::StateViewConcept<Kind> PlanningState,
             ygg::formalism::RelationBindingViewConcept<tyr::formalism::planning::Action<tyr::LiftedTag>, tyr::formalism::ObjectTag> Binding>
    bool matches(RuleEvaluationContext<ExtFamilyTag, Kind, Storage, PlanningState>& context,
                 State state,
                 const tyr::planning::LabeledNode<Kind, PlanningState, Binding>& candidate) const
    {
        const auto& planning_state = context.planning_state;
        const auto rule = m_rule;
        const auto action = candidate.label.get_relation();
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment) || &action.get_context() != &m_action.get_context()
            || action != m_action)
            return false;
        auto state_context = context.make_dl_context(state);
        const auto query = evaluate<Kind>(rule.get_query_feature(), state_context);
        validate_query(planning_state, query.arity());
        const auto objects = candidate.label.get_objects().get_data();
        if (objects.size() != query.arity() || !query.contains(objects))
            return false;
        auto& generator = *context.task_context->search_context->successor_generator;
        const auto node = tyr::planning::Node(planning_state, 0);
        const auto status = [&]
        {
            if constexpr (std::same_as<Binding, tyr::formalism::planning::ActionBindingView>)
                return generator.check_action_binding(node, candidate.label);
            else
                return generator.check_action_binding(node, candidate.label.get_relation(), candidate.label.get_objects());
        }();
        require_applicable(status, planning_state, objects);
        check_action_effects(context, rule, state, planning_state, candidate.node, objects);
        return true;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
