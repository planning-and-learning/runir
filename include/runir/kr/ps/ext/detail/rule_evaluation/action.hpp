#ifndef RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_ACTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_RULE_EVALUATION_ACTION_HPP_

#include "runir/kr/ps/dl/evaluation.hpp"
#include "runir/kr/ps/ext/action_rule_contract_error.hpp"
#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/execution_storage.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"

#include <algorithm>
#include <concepts>
#include <optional>
#include <span>
#include <sstream>
#include <string_view>
#include <tyr/formalism/planning/grounder_decl.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/action_executor.hpp>
#include <tyr/planning/applicability.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <tyr/planning/state_view.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>

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

template<tyr::TaskKind Kind>
class ActionRuleWorkspace
{
public:
    using Action = tyr::formalism::planning::ActionView<tyr::LiftedTag>;
    using GroundAction = tyr::formalism::planning::ActionView<tyr::GroundTag>;
    using Binding = tyr::formalism::planning::ActionBindingView;
    struct ActionInfo
    {
        Action schema;
        std::vector<ygg::UnorderedSet<ygg::Index<tyr::formalism::Object>>> parameter_domains;
    };

private:
    tyr::planning::TaskPtr<Kind> m_task;
    tyr::formalism::planning::Builder m_builder;
    ygg::Data<tyr::formalism::RelationBinding<tyr::formalism::planning::Action<tyr::LiftedTag>>> m_binding;
    tyr::planning::ActionExecutor m_executor;
    std::vector<ActionInfo> m_schemas;
    ygg::UnorderedMap<Binding, GroundAction> m_ground_actions;
    bool m_ground_actions_ready = false;

public:
    std::vector<ygg::uint_t> tuple;
    explicit ActionRuleWorkspace(tyr::planning::TaskPtr<Kind> task) : m_task(std::move(task)) {}

    size_t prepare(Action schema)
    {
        for (size_t i = 0; i < m_schemas.size(); ++i)
            if (m_schemas[i].schema == schema)
                return i;
        auto info = ActionInfo { schema, {} };
        if constexpr (std::same_as<Kind, tyr::LiftedTag>)
        {
            const auto& domains = m_task->get_formalism_task().get_variable_domains().action_domains.at(schema.get_index()).payload.precondition_domain.payload;
            info.parameter_domains.reserve(domains.size());
            for (const auto& domain : domains)
                info.parameter_domains.emplace_back(domain.objects.begin(), domain.objects.end());
        }
        m_schemas.push_back(std::move(info));
        return m_schemas.size() - 1;
    }

    template<tyr::planning::StateViewConcept<Kind> State>
    Binding applicable_binding(size_t slot, RuleView<ActionTag> rule, const State& planning_state, std::span<const ygg::uint_t> tuple)
    {
        const auto& info = m_schemas[slot];
        const auto schema = info.schema;
        const auto& domains = info.parameter_domains;
        m_binding.relation = schema.get_index();
        m_binding.objects.clear();
        for (size_t i = 0; i < tuple.size(); ++i)
        {
            const auto object = ygg::Index<tyr::formalism::Object>(tuple[i]);
            if constexpr (std::same_as<Kind, tyr::LiftedTag>)
                if (i >= domains.size() || !domains[i].contains(object))
                    action_rule_contract_error(rule, planning_state, tuple, "object is outside the action parameter domain");
            m_binding.objects.push_back(object);
        }
        // Policy applicability depends on the planning state, without a carried path metric.
        const auto state = tyr::planning::StateContext<Kind>(*m_task, planning_state.get_state_builder(), 0);
        if constexpr (std::same_as<Kind, tyr::GroundTag>)
        {
            if (!m_ground_actions_ready)
            {
                for (const auto ground_action : m_task->get_task().get_ground_actions())
                    m_ground_actions.emplace(ground_action.get_row(), ground_action);
                m_ground_actions_ready = true;
            }
            const auto binding = tyr::formalism::planning::get_or_create(*m_task->get_repository(), m_binding).first;
            const auto it = m_ground_actions.find(binding);
            if (it == m_ground_actions.end() || !m_executor.is_applicable(it->second, state))
                action_rule_contract_error(rule, planning_state, tuple, "offered action is not applicable");
            return binding;
        }
        else
        {
            auto grounder = tyr::formalism::planning::GrounderContext { m_builder, *m_task->get_repository(), m_binding.objects };
            if (!m_executor.is_applicable(schema, state, grounder, *m_task->get_fdr_context()))
                action_rule_contract_error(rule, planning_state, tuple, "offered action is not applicable");
            return tyr::formalism::planning::get_or_create(*m_task->get_repository(), m_binding).first;
        }
    }
};

template<tyr::TaskKind Kind>
class ActionRuleEvaluator
{
    RuleView<ActionTag> m_rule;
    RuleVariantView m_variant;
    std::optional<tyr::formalism::planning::ActionView<tyr::LiftedTag>> m_action;
    std::optional<size_t> m_action_slot;

    template<tyr::planning::StateViewConcept<Kind> State>
    void prepare(ActionRuleWorkspace<Kind>& workspace, const State& state, size_t arity)
    {
        // Unvisited rules and query rows must not trigger contract errors.
        if (!m_action)
            action_rule_contract_error(m_rule, state, {}, "action schema does not exist in the task");
        if (m_action->get_arity() != arity)
            action_rule_contract_error(m_rule, state, {}, "query arity does not match action arity");
        if (!m_action_slot)
            m_action_slot = workspace.prepare(*m_action);
    }

public:
    using RuleTag = ActionTag;
    ActionRuleEvaluator(RuleView<ActionTag> rule, RuleVariantView variant, const tyr::planning::Task<Kind>& task) : m_rule(rule), m_variant(variant)
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
        for (const auto effect : rule.get_effects())
            if (!is_compatible_with(effect, transition))
                detail::action_rule_contract_error(rule, planning_state, tuple, "offered transition violates declared effects");
    }

public:
    template<typename Context, typename Emit, typename Stop, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool emit(Context& context, S state, const PS& planning_state, Emit&& emit, Stop&& stop, ActionRuleWorkspace<Kind>& workspace)
    {
        const auto rule = m_rule;
        const auto rule_variant = m_variant;
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment))
            return true;
        auto state_context =
            context.environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        const auto query = evaluate(rule.get_query_feature(), state_context);
        prepare(workspace, planning_state, query.arity());
        for (std::size_t i = 0; i < query.size(); ++i)
        {
            if (stop())
                return false;
            const auto tuple = query[i];
            const auto binding = workspace.applicable_binding(*m_action_slot, rule, planning_state, tuple);
            const auto candidate = context.storage.successor(planning_state, binding);
            check_action_effects(context, rule, state, planning_state, candidate, tuple);
            if (!emit(detail::planning_step(context.storage, state, candidate, rule_variant, rule.get_target(), context.task_context)))
                return false;
        }
        return true;
    }
    template<typename Context, ProgramStateViewConcept<Kind> S, tyr::planning::StateViewConcept<Kind> PS>
    bool matches(Context& context, S state, const PS& planning_state, const tyr::planning::LabeledNode<PS>& candidate, ActionRuleWorkspace<Kind>& workspace)
    {
        const auto rule = m_rule;
        if (!ext::rule_is_applicable(rule, state, planning_state, context.environment) || candidate.label.get_relation().get_name() != rule.get_action_name())
            return false;
        auto state_context =
            context.environment.make_dl_context(planning_state, state.get_module_state().get_arguments(), state.get_module_state().get_registers());
        const auto query = evaluate(rule.get_query_feature(), state_context);
        prepare(workspace, planning_state, query.arity());
        workspace.tuple.clear();
        for (const auto object : candidate.label.get_objects())
            workspace.tuple.push_back(ygg::uint_t(object.get_index()));
        if (workspace.tuple.size() != query.arity() || !query.contains(std::span<const ygg::uint_t>(workspace.tuple)))
            return false;
        workspace.applicable_binding(*m_action_slot, rule, planning_state, workspace.tuple);
        check_action_effects(context, rule, state, planning_state, candidate, workspace.tuple);
        return true;
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
