#ifndef RUNIR_KR_PS_EXT_DETAIL_ACTION_RULE_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_ACTION_RULE_HPP_

#include "runir/kr/ps/ext/action_rule_contract_error.hpp"
#include "runir/kr/ps/ext/repository.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"

#include <algorithm>
#include <concepts>
#include <span>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>
#include <tyr/formalism/planning/grounder_decl.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/planning/action_executor.hpp>
#include <tyr/planning/applicability.hpp>
#include <tyr/planning/ground/task.hpp>
#include <tyr/planning/lifted/task.hpp>
#include <tyr/planning/node.hpp>
#include <yggdrasil/containers/associative_containers.hpp>

namespace runir::kr::ps::ext::detail
{

template<tyr::TaskKind Kind>
[[noreturn]] void action_rule_contract_error(RuleView<ActionTag> rule,
                                             const tyr::planning::StateView<Kind>& state,
                                             std::span<const ygg::uint_t> tuple,
                                             std::string_view reason)
{
    auto message = std::ostringstream {};
    message << "Action rule " << ygg::uint_t(rule.get_index()) << " for '" << rule.get_action_name().str()
            << "' in state " << ygg::uint_t(state.get_index()) << " with tuple (";
    for (size_t i = 0; i < tuple.size(); ++i)
        message << (i ? ", " : "") << tuple[i];
    message << "): " << reason;
    throw ActionRuleContractError(message.str());
}

template<tyr::TaskKind Kind>
class ActionRuleEvaluator
{
    using Action = tyr::formalism::planning::ActionView<tyr::LiftedTag>;
    using GroundAction = tyr::formalism::planning::ActionView<tyr::GroundTag>;
    using Binding = tyr::formalism::planning::ActionBindingView;

    struct ActionInfo
    {
        Action schema;
        std::vector<ygg::UnorderedSet<ygg::Index<tyr::formalism::Object>>> parameter_domains;
    };

    tyr::planning::TaskPtr<Kind> m_task;
    tyr::formalism::planning::Builder m_builder;
    ygg::Data<tyr::formalism::RelationBinding<tyr::formalism::planning::Action<tyr::LiftedTag>>> m_binding;
    tyr::planning::ActionExecutor m_executor;
    ygg::UnorderedMap<ygg::Index<Rule<ActionTag>>, ActionInfo> m_actions;
    ygg::UnorderedMap<Binding, GroundAction> m_ground_actions;
    bool m_ground_actions_ready = false;

public:
    explicit ActionRuleEvaluator(tyr::planning::TaskPtr<Kind> task) : m_task(std::move(task)) {}

    const ActionInfo& action_info(RuleView<ActionTag> rule, const tyr::planning::StateView<Kind>& state, size_t arity)
    {
        auto it = m_actions.find(rule.get_index());
        if (it == m_actions.end())
        {
            for (const auto schema : m_task->get_task().get_domain().get_actions())
                if (schema.get_name().str() == rule.get_action_name())
                {
                    auto info = ActionInfo { schema, {} };
                    if constexpr (std::same_as<Kind, tyr::LiftedTag>)
                    {
                        const auto& domains = m_task->get_formalism_task().get_variable_domains().action_domains.at(schema.get_index()).payload.precondition_domain.payload;
                        info.parameter_domains.reserve(domains.size());
                        for (const auto& domain : domains)
                            info.parameter_domains.emplace_back(domain.objects.begin(), domain.objects.end());
                    }
                    it = m_actions.emplace(rule.get_index(), std::move(info)).first;
                    break;
                }
            if (it == m_actions.end())
                action_rule_contract_error(rule, state, {}, "action schema does not exist in the task");
        }
        if (it->second.schema.get_arity() != arity)
            action_rule_contract_error(rule, state, {}, "query arity does not match action arity");
        return it->second;
    }

    Action action(RuleView<ActionTag> rule, const tyr::planning::StateView<Kind>& state, size_t arity)
    {
        return action_info(rule, state, arity).schema;
    }

    Binding applicable_binding(RuleView<ActionTag> rule, const tyr::planning::Node<Kind>& node, std::span<const ygg::uint_t> tuple)
    {
        const auto& info = action_info(rule, node.get_state(), tuple.size());
        const auto schema = info.schema;
        const auto& domains = info.parameter_domains;
        m_binding.relation = schema.get_index();
        m_binding.objects.clear();
        for (size_t i = 0; i < tuple.size(); ++i)
        {
            const auto object = ygg::Index<tyr::formalism::Object>(tuple[i]);
            if constexpr (std::same_as<Kind, tyr::LiftedTag>)
                if (i >= domains.size() || !domains[i].contains(object))
                    action_rule_contract_error(rule, node.get_state(), tuple, "object is outside the action parameter domain");
            m_binding.objects.push_back(object);
        }
        const auto state = tyr::planning::StateContext<Kind>(*m_task, node.get_state().get_state_builder(), node.get_metric());
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
                action_rule_contract_error(rule, node.get_state(), tuple, "offered action is not applicable");
            return binding;
        }
        else
        {
            auto grounder = tyr::formalism::planning::GrounderContext { m_builder, *m_task->get_repository(), m_binding.objects };
            if (!m_executor.is_applicable(schema, state, grounder, *m_task->get_fdr_context()))
                action_rule_contract_error(rule, node.get_state(), tuple, "offered action is not applicable");
            return tyr::formalism::planning::get_or_create(*m_task->get_repository(), m_binding).first;
        }
    }
};

}  // namespace runir::kr::ps::ext::detail

#endif
