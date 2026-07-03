#ifndef RUNIR_KR_PS_EXT_DETAIL_EXECUTION_HPP_
#define RUNIR_KR_PS_EXT_DETAIL_EXECUTION_HPP_

#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/evaluation.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"

#include <concepts>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext::detail
{

enum class RuleExecutionStatus
{
    APPLIED,
    NOT_APPLICABLE,
    EMPTY_DENOTATION,
    NO_APPLICABLE_ACTION,
    MALFORMED_CALL
};

template<runir::kr::dl::CategoryTag Category, typename C, tyr::planning::TaskKind Kind>
bool has_current_source(ygg::View<ygg::Index<Rule<LoadTag, Category>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_memory_state().get_index();
}

template<typename C, tyr::planning::TaskKind Kind>
bool has_current_source(ygg::View<ygg::Index<Rule<SketchTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_memory_state().get_index();
}

template<typename C, tyr::planning::TaskKind Kind>
bool has_current_source(ygg::View<ygg::Index<Rule<DoTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_memory_state().get_index();
}

template<typename C, tyr::planning::TaskKind Kind>
bool has_current_source(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_memory_state().get_index();
}

template<runir::kr::dl::CategoryTag Category, typename C, tyr::planning::TaskKind Kind>
RuleExecutionStatus
execute_load(ygg::View<ygg::Index<Rule<LoadTag, Category>>, C> rule, EvaluationContext<Kind>& context, EvaluationEnvironment<Kind>& environment)
{
    if (!has_current_source(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    if (!conditions_are_compatible(rule, context, environment))
        return RuleExecutionStatus::NOT_APPLICABLE;

    const auto denotation = evaluate(rule.get_expression(), context, environment);
    const auto first = denotation.begin();
    if (first == denotation.end())
        return RuleExecutionStatus::EMPTY_DENOTATION;

    if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
        context.set(rule.get_register().get_identifier(), (*first).get_index());
    else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
        context.set(rule.get_register().get_identifier(), (*first).first.get_index(), (*first).second.get_index());
    else
        static_assert(ygg::dependent_false<Category>::value, "unhandled load rule category");
    context.set_memory_state(rule.get_target());
    return RuleExecutionStatus::APPLIED;
}

template<typename C, tyr::planning::TaskKind Kind>
auto evaluate_do_arguments(ygg::View<ygg::Index<Rule<DoTag>>, C> rule, EvaluationContext<Kind>& context, EvaluationEnvironment<Kind>& environment)
{
    auto action_arguments = rule.get_action_arguments();
    auto denotations = std::vector<decltype(evaluate(action_arguments.front(), context, environment))> {};
    denotations.reserve(action_arguments.size());
    for (auto argument : action_arguments)
        denotations.push_back(evaluate(argument, context, environment));
    return denotations;
}

template<typename ConceptDenotations, typename C>
bool action_matches_do_arguments(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                                 tyr::formalism::planning::GroundActionView action,
                                 const ConceptDenotations& denotations)
{
    if (action.get_action().get_name() != rule.get_action_name())
        return false;

    const auto objects = action.get_row().get_objects();
    if (objects.size() != denotations.size())
        return false;

    for (size_t index = 0; index < denotations.size(); ++index)
        if (!denotations[index].test(ygg::uint_t(objects[index].get_index())))
            return false;

    return true;
}

// Whether a Do rule selects the concrete transition (context state -> target_state) taken by
// `action`: memory match + conditions on the source + action name/argument match + effects
// across the transition. The single source of truth shared by the executor (select_do_successor)
// and the public successor expander (ext/successor_expander.hpp).
template<typename C, tyr::planning::TaskKind Kind>
bool do_rule_matches(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                     EvaluationContext<Kind>& context,
                     EvaluationEnvironment<Kind>& environment,
                     tyr::formalism::planning::GroundActionView action,
                     const tyr::planning::StateView<Kind>& target_state)
{
    if (!has_current_source(rule, context))
        return false;

    if (!conditions_are_compatible(rule, context, environment))
        return false;

    const auto denotations = evaluate_do_arguments(rule, context, environment);
    if (!action_matches_do_arguments(rule, action, denotations))
        return false;

    auto dl_context = environment.make_dl_transition_context(context, target_state);
    return is_compatible_with(rule, dl_context);
}

template<typename C, tyr::planning::TaskKind Kind>
std::optional<tyr::planning::LabeledNode<Kind>> select_do_successor(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                                                                    EvaluationContext<Kind>& context,
                                                                    EvaluationEnvironment<Kind>& environment,
                                                                    const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    for (const auto& successor : successors)
        if (do_rule_matches(rule, context, environment, successor.label, successor.node.get_state()))
            return successor;

    return std::nullopt;
}

template<typename C, tyr::planning::TaskKind Kind>
RuleExecutionStatus execute_do(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                               EvaluationContext<Kind>& context,
                               EvaluationEnvironment<Kind>& environment,
                               const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    if (!has_current_source(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    if (!conditions_are_compatible(rule, context, environment))
        return RuleExecutionStatus::NOT_APPLICABLE;

    const auto successor = select_do_successor(rule, context, environment, successors);
    if (!successor)
        return RuleExecutionStatus::NO_APPLICABLE_ACTION;

    context.set_state(successor->node.get_state());
    context.set_memory_state(rule.get_target());
    return RuleExecutionStatus::APPLIED;
}

template<typename C, tyr::planning::TaskKind Kind>
bool sketch_rule_matches_state(ygg::View<ygg::Index<Rule<SketchTag>>, C> rule,
                               EvaluationContext<Kind>& context,
                               EvaluationEnvironment<Kind>& environment,
                               const tyr::planning::StateView<Kind>& target_state)
{
    if (!has_current_source(rule, context))
        return false;

    auto dl_context = environment.make_dl_transition_context(context, target_state);
    return is_compatible_with(rule, dl_context);
}

template<tyr::planning::TaskKind Kind>
std::optional<RuleVariantView> find_matching_sketch_rule_variant(EvaluationContext<Kind>& context,
                                                                 EvaluationEnvironment<Kind>& environment,
                                                                 const tyr::planning::StateView<Kind>& target_state)
{
    const auto module = context.get_module();
    for (const auto& transition : module.get_memory_transitions())
    {
        for (auto rule : transition)
        {
            const auto matches = ygg::visit(
                [&](auto concrete_rule)
                {
                    using RuleView = std::decay_t<decltype(concrete_rule)>;
                    if constexpr (std::same_as<RuleView, ygg::View<ygg::Index<Rule<SketchTag>>, Repository>>)
                        return sketch_rule_matches_state(concrete_rule, context, environment, target_state);
                    else
                        return false;
                },
                rule.get_variant());
            if (matches)
                return rule;
        }
    }

    return std::nullopt;
}

template<tyr::planning::TaskKind Kind>
std::optional<ygg::View<ygg::Index<Rule<SketchTag>>, Repository>>
find_matching_sketch_rule(EvaluationContext<Kind>& context, EvaluationEnvironment<Kind>& environment, const tyr::planning::StateView<Kind>& target_state)
{
    const auto rule_variant = find_matching_sketch_rule_variant(context, environment, target_state);
    if (!rule_variant)
        return std::nullopt;

    return ygg::visit(
        [](auto concrete_rule) -> std::optional<ygg::View<ygg::Index<Rule<SketchTag>>, Repository>>
        {
            using RuleView = std::decay_t<decltype(concrete_rule)>;
            if constexpr (std::same_as<RuleView, ygg::View<ygg::Index<Rule<SketchTag>>, Repository>>)
                return concrete_rule;
            else
                return std::nullopt;
        },
        rule_variant->get_variant());
}

template<typename C, tyr::planning::TaskKind Kind>
bool sketch_rule_matches_successor(ygg::View<ygg::Index<Rule<SketchTag>>, C> rule,
                                   EvaluationContext<Kind>& context,
                                   EvaluationEnvironment<Kind>& environment,
                                   const tyr::planning::LabeledNode<Kind>& successor)
{
    return sketch_rule_matches_state(rule, context, environment, successor.node.get_state());
}

template<typename C, tyr::planning::TaskKind Kind>
std::optional<tyr::planning::LabeledNode<Kind>> select_sketch_successor(ygg::View<ygg::Index<Rule<SketchTag>>, C> rule,
                                                                        EvaluationContext<Kind>& context,
                                                                        EvaluationEnvironment<Kind>& environment,
                                                                        const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    for (const auto& successor : successors)
        if (sketch_rule_matches_successor(rule, context, environment, successor))
            return successor;

    return std::nullopt;
}

template<typename C, tyr::planning::TaskKind Kind>
RuleExecutionStatus execute_sketch(ygg::View<ygg::Index<Rule<SketchTag>>, C> rule,
                                   EvaluationContext<Kind>& context,
                                   EvaluationEnvironment<Kind>& environment,
                                   const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    if (!has_current_source(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    if (rule.get_effects().empty() && conditions_are_compatible(rule, context, environment))
    {
        context.set_memory_state(rule.get_target());
        return RuleExecutionStatus::APPLIED;
    }

    const auto successor = select_sketch_successor(rule, context, environment, successors);
    if (!successor)
        return RuleExecutionStatus::NOT_APPLICABLE;

    context.set_state(successor->node.get_state());
    context.set_memory_state(rule.get_target());
    return RuleExecutionStatus::APPLIED;
}

template<tyr::planning::TaskKind Kind>
struct EvaluatedCallArguments
{
    typename EvaluationContext<Kind>::template Arguments<runir::kr::dl::ConceptTag> concept_arguments;
    typename EvaluationContext<Kind>::template Arguments<runir::kr::dl::RoleTag> role_arguments;
    typename EvaluationContext<Kind>::template Arguments<runir::kr::dl::BooleanTag> boolean_arguments;
    typename EvaluationContext<Kind>::template Arguments<runir::kr::dl::NumericalTag> numerical_arguments;
};

template<tyr::planning::TaskKind Kind>
void append_call_argument(ConceptArgument argument,
                          EvaluationContext<Kind>& context,
                          EvaluationEnvironment<Kind>& environment,
                          EvaluatedCallArguments<Kind>& target)
{
    target.concept_arguments.push_back(evaluate_argument(argument, context, environment));
}

template<tyr::planning::TaskKind Kind>
void append_call_argument(RoleArgument argument,
                          EvaluationContext<Kind>& context,
                          EvaluationEnvironment<Kind>& environment,
                          EvaluatedCallArguments<Kind>& target)
{
    target.role_arguments.push_back(evaluate_argument(argument, context, environment));
}

template<tyr::planning::TaskKind Kind>
void append_call_argument(BooleanArgument argument,
                          EvaluationContext<Kind>& context,
                          EvaluationEnvironment<Kind>& environment,
                          EvaluatedCallArguments<Kind>& target)
{
    target.boolean_arguments.push_back(evaluate_argument(argument, context, environment));
}

template<tyr::planning::TaskKind Kind>
void append_call_argument(NumericalArgument argument,
                          EvaluationContext<Kind>& context,
                          EvaluationEnvironment<Kind>& environment,
                          EvaluatedCallArguments<Kind>& target)
{
    target.numerical_arguments.push_back(evaluate_argument(argument, context, environment));
}

template<typename C, tyr::planning::TaskKind Kind>
auto evaluate_call_arguments(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, EvaluationContext<Kind>& context, EvaluationEnvironment<Kind>& environment)
{
    auto result = EvaluatedCallArguments<Kind> {};
    for (const auto& argument : rule.get_call_arguments())
        argument.apply([&](auto typed_argument) { append_call_argument(typed_argument, context, environment, result); });
    return result;
}

template<tyr::planning::TaskKind Kind>
bool call_arguments_match_signature(ModuleView callee, const EvaluatedCallArguments<Kind>& arguments)
{
    return arguments.concept_arguments.size() == callee.template get_arguments<runir::kr::dl::ConceptTag>().size()
           && arguments.role_arguments.size() == callee.template get_arguments<runir::kr::dl::RoleTag>().size()
           && arguments.boolean_arguments.size() == callee.template get_arguments<runir::kr::dl::BooleanTag>().size()
           && arguments.numerical_arguments.size() == callee.template get_arguments<runir::kr::dl::NumericalTag>().size();
}

template<typename C, tyr::planning::TaskKind Kind>
std::optional<ModuleView> resolve_callee(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    if (rule.get_data().callee_name.size() != 0)
        return context.find_module(rule.get_callee_name());
    return ModuleView(rule.get_data().callee, context.get_repository());
}

template<typename C, tyr::planning::TaskKind Kind>
RuleExecutionStatus execute_call(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, EvaluationContext<Kind>& context, EvaluationEnvironment<Kind>& environment)
{
    if (!has_current_source(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    if (!conditions_are_compatible(rule, context, environment))
        return RuleExecutionStatus::NOT_APPLICABLE;

    auto arguments = evaluate_call_arguments(rule, context, environment);
    const auto callee = resolve_callee(rule, context);
    if (!callee || !call_arguments_match_signature(*callee, arguments))
        return RuleExecutionStatus::MALFORMED_CALL;

    context.enter_module(*callee,
                         rule.get_target(),
                         std::move(arguments.concept_arguments),
                         std::move(arguments.role_arguments),
                         std::move(arguments.boolean_arguments),
                         std::move(arguments.numerical_arguments));
    return RuleExecutionStatus::APPLIED;
}

}  // namespace runir::kr::ps::ext::detail

#endif
