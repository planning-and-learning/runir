#ifndef RUNIR_KR_PS_EXT_EXECUTION_HPP_
#define RUNIR_KR_PS_EXT_EXECUTION_HPP_

#include "runir/kr/ps/ext/compatibility.hpp"
#include "runir/kr/ps/ext/evaluation.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"

#include <concepts>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>
#include <tyr/planning/declarations.hpp>
#include <tyr/planning/node.hpp>
#include <utility>
#include <vector>

namespace runir::kr::ps::ext
{

enum class RuleExecutionStatus
{
    APPLIED,
    NOT_APPLICABLE,
    EMPTY_DENOTATION,
    NO_APPLICABLE_ACTION,
    MALFORMED_CALL,
    NO_APPLICABLE_RULE,
    RETURNED_FROM_CALL
};

enum class LoadExecutionStatus
{
    STABLE,
    APPLIED,
    EMPTY_DENOTATION
};

template<typename C, tyr::planning::TaskKind Kind>
bool has_current_source(tyr::View<tyr::Index<Rule<LoadTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_memory_state().get_index();
}

template<typename C, tyr::planning::TaskKind Kind>
bool has_current_source(tyr::View<tyr::Index<Rule<SketchTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_memory_state().get_index();
}

template<typename C, tyr::planning::TaskKind Kind>
bool has_current_source(tyr::View<tyr::Index<Rule<DoTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_memory_state().get_index();
}

template<typename C, tyr::planning::TaskKind Kind>
bool has_current_source(tyr::View<tyr::Index<Rule<CallTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_memory_state().get_index();
}

template<typename C, tyr::planning::TaskKind Kind>
RuleExecutionStatus execute_load(tyr::View<tyr::Index<Rule<LoadTag>>, C> rule, EvaluationContext<Kind>& context)
{
    if (!has_current_source(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    if (!conditions_are_compatible(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    const auto denotation = evaluate(rule.get_concept(), context);
    const auto first = denotation.begin();
    if (first == denotation.end())
        return RuleExecutionStatus::EMPTY_DENOTATION;

    context.set(rule.get_register().get_identifier(), (*first).get_index());
    context.set_memory_state(rule.get_target());
    return RuleExecutionStatus::APPLIED;
}

template<typename C, tyr::planning::TaskKind Kind>
RuleExecutionStatus execute_load(tyr::View<tyr::Index<RuleVariant>, C> rule, EvaluationContext<Kind>& context)
{
    return tyr::visit(
        [&](auto concrete_rule)
        {
            using RuleView = std::decay_t<decltype(concrete_rule)>;
            if constexpr (std::same_as<RuleView, tyr::View<tyr::Index<Rule<LoadTag>>, C>>)
                return execute_load(concrete_rule, context);
            else
                return RuleExecutionStatus::NOT_APPLICABLE;
        },
        rule.get_variant());
}

template<tyr::planning::TaskKind Kind>
LoadExecutionStatus execute_next_load(EvaluationContext<Kind>& context)
{
    const auto module = context.get_module();
    for (const auto& transition : module.get_memory_transitions())
    {
        if (transition.source != context.get_memory_state().get_index())
            continue;

        for (auto rule : tyr::make_view(transition.rules, context.get_repository()))
        {
            const auto status = execute_load(rule, context);
            if (status == RuleExecutionStatus::APPLIED)
                return LoadExecutionStatus::APPLIED;
            if (status == RuleExecutionStatus::EMPTY_DENOTATION)
                return LoadExecutionStatus::EMPTY_DENOTATION;
        }
    }

    return LoadExecutionStatus::STABLE;
}

template<typename C, tyr::planning::TaskKind Kind>
auto evaluate_do_arguments(tyr::View<tyr::Index<Rule<DoTag>>, C> rule, EvaluationContext<Kind>& context)
{
    auto denotations = std::vector<decltype(evaluate_argument(rule.get_data().arguments.front(), context))> {};
    denotations.reserve(rule.get_data().arguments.size());
    for (auto argument : rule.get_data().arguments)
        denotations.push_back(evaluate_argument(argument, context));
    return denotations;
}

template<typename ConceptDenotations, typename C>
bool action_matches_do_arguments(tyr::View<tyr::Index<Rule<DoTag>>, C> rule,
                                 tyr::formalism::planning::GroundActionView action,
                                 const ConceptDenotations& denotations)
{
    if (action.get_action().get_name() != rule.get_action_name())
        return false;

    const auto objects = action.get_row().get_objects();
    if (objects.size() != denotations.size())
        return false;

    for (size_t index = 0; index < denotations.size(); ++index)
        if (!denotations[index].get().test(tyr::uint_t(objects[index].get_index())))
            return false;

    return true;
}

template<typename C, tyr::planning::TaskKind Kind>
std::optional<tyr::planning::LabeledNode<Kind>> select_do_successor(tyr::View<tyr::Index<Rule<DoTag>>, C> rule,
                                                                    EvaluationContext<Kind>& context,
                                                                    const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    if (!has_current_source(rule, context))
        return std::nullopt;

    if (!conditions_are_compatible(rule, context))
        return std::nullopt;

    const auto denotations = evaluate_do_arguments(rule, context);
    for (const auto& successor : successors)
        if (action_matches_do_arguments(rule, successor.label, denotations))
            return successor;

    return std::nullopt;
}

template<typename C, tyr::planning::TaskKind Kind>
RuleExecutionStatus
execute_do(tyr::View<tyr::Index<Rule<DoTag>>, C> rule, EvaluationContext<Kind>& context, const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    if (!has_current_source(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    if (!conditions_are_compatible(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    const auto successor = select_do_successor(rule, context, successors);
    if (!successor)
        return RuleExecutionStatus::NO_APPLICABLE_ACTION;

    context.set_state(successor->node.get_state());
    context.set_memory_state(rule.get_target());
    return RuleExecutionStatus::APPLIED;
}

template<typename C, tyr::planning::TaskKind Kind>
bool sketch_rule_matches_state(tyr::View<tyr::Index<Rule<SketchTag>>, C> rule,
                               EvaluationContext<Kind>& context,
                               const tyr::planning::StateView<Kind>& target_state)
{
    if (!has_current_source(rule, context))
        return false;

    return context.with_dl_transition_context(target_state, [&](auto& dl_context) { return is_compatible_with(rule, dl_context); });
}

template<tyr::planning::TaskKind Kind>
std::optional<RuleVariantView> find_matching_sketch_rule_variant(EvaluationContext<Kind>& context, const tyr::planning::StateView<Kind>& target_state)
{
    const auto module = context.get_module();
    for (const auto& transition : module.get_memory_transitions())
    {
        if (transition.source != context.get_memory_state().get_index())
            continue;

        for (auto rule : tyr::make_view(transition.rules, context.get_repository()))
        {
            const auto matches = tyr::visit(
                [&](auto concrete_rule)
                {
                    using RuleView = std::decay_t<decltype(concrete_rule)>;
                    if constexpr (std::same_as<RuleView, tyr::View<tyr::Index<Rule<SketchTag>>, Repository>>)
                        return sketch_rule_matches_state(concrete_rule, context, target_state);
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
std::optional<tyr::View<tyr::Index<Rule<SketchTag>>, Repository>> find_matching_sketch_rule(EvaluationContext<Kind>& context,
                                                                                            const tyr::planning::StateView<Kind>& target_state)
{
    const auto rule_variant = find_matching_sketch_rule_variant(context, target_state);
    if (!rule_variant)
        return std::nullopt;

    return tyr::visit(
        [](auto concrete_rule) -> std::optional<tyr::View<tyr::Index<Rule<SketchTag>>, Repository>>
        {
            using RuleView = std::decay_t<decltype(concrete_rule)>;
            if constexpr (std::same_as<RuleView, tyr::View<tyr::Index<Rule<SketchTag>>, Repository>>)
                return concrete_rule;
            else
                return std::nullopt;
        },
        rule_variant->get_variant());
}

template<typename C, tyr::planning::TaskKind Kind>
bool sketch_rule_matches_successor(tyr::View<tyr::Index<Rule<SketchTag>>, C> rule,
                                   EvaluationContext<Kind>& context,
                                   const tyr::planning::LabeledNode<Kind>& successor)
{
    return sketch_rule_matches_state(rule, context, successor.node.get_state());
}

template<typename C, tyr::planning::TaskKind Kind>
std::optional<tyr::planning::LabeledNode<Kind>> select_sketch_successor(tyr::View<tyr::Index<Rule<SketchTag>>, C> rule,
                                                                        EvaluationContext<Kind>& context,
                                                                        const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    for (const auto& successor : successors)
        if (sketch_rule_matches_successor(rule, context, successor))
            return successor;

    return std::nullopt;
}

template<tyr::planning::TaskKind Kind>
std::optional<RuleVariantView> find_applicable_sketch_rule(EvaluationContext<Kind>& context,
                                                           const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    const auto module = context.get_module();
    for (const auto& transition : module.get_memory_transitions())
    {
        if (transition.source != context.get_memory_state().get_index())
            continue;

        for (auto rule : tyr::make_view(transition.rules, context.get_repository()))
        {
            const auto applicable = tyr::visit(
                [&](auto concrete_rule)
                {
                    using RuleView = std::decay_t<decltype(concrete_rule)>;
                    if constexpr (std::same_as<RuleView, tyr::View<tyr::Index<Rule<SketchTag>>, Repository>>)
                        return (concrete_rule.get_effects().empty() && conditions_are_compatible(concrete_rule, context))
                               || select_sketch_successor(concrete_rule, context, successors).has_value();
                    else
                        return false;
                },
                rule.get_variant());
            if (applicable)
                return rule;
        }
    }

    return std::nullopt;
}

template<typename C, tyr::planning::TaskKind Kind>
RuleExecutionStatus execute_sketch(tyr::View<tyr::Index<Rule<SketchTag>>, C> rule,
                                   EvaluationContext<Kind>& context,
                                   const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    if (!has_current_source(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    if (rule.get_effects().empty() && conditions_are_compatible(rule, context))
    {
        context.set_memory_state(rule.get_target());
        return RuleExecutionStatus::APPLIED;
    }

    const auto successor = select_sketch_successor(rule, context, successors);
    if (!successor)
        return RuleExecutionStatus::NOT_APPLICABLE;

    context.set_state(successor->node.get_state());
    context.set_memory_state(rule.get_target());
    return RuleExecutionStatus::APPLIED;
}

template<tyr::planning::TaskKind Kind>
RuleExecutionStatus execute_next_sketch_rule(EvaluationContext<Kind>& context, const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    const auto module = context.get_module();
    for (const auto& transition : module.get_memory_transitions())
    {
        if (transition.source != context.get_memory_state().get_index())
            continue;

        for (auto rule : tyr::make_view(transition.rules, context.get_repository()))
        {
            const auto status = tyr::visit(
                [&](auto concrete_rule)
                {
                    using RuleView = std::decay_t<decltype(concrete_rule)>;
                    if constexpr (std::same_as<RuleView, tyr::View<tyr::Index<Rule<SketchTag>>, Repository>>)
                        return execute_sketch(concrete_rule, context, successors);
                    else
                        return RuleExecutionStatus::NOT_APPLICABLE;
                },
                rule.get_variant());
            if (status != RuleExecutionStatus::NOT_APPLICABLE)
                return status;
        }
    }

    return RuleExecutionStatus::NO_APPLICABLE_RULE;
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
void append_call_argument(ConceptArgument argument, EvaluationContext<Kind>& context, EvaluatedCallArguments<Kind>& target)
{
    target.concept_arguments.push_back(evaluate_argument(argument, context));
}

template<tyr::planning::TaskKind Kind>
void append_call_argument(RoleArgument argument, EvaluationContext<Kind>& context, EvaluatedCallArguments<Kind>& target)
{
    target.role_arguments.push_back(evaluate_argument(argument, context));
}

template<tyr::planning::TaskKind Kind>
void append_call_argument(BooleanArgument argument, EvaluationContext<Kind>& context, EvaluatedCallArguments<Kind>& target)
{
    target.boolean_arguments.push_back(evaluate_argument(argument, context));
}

template<tyr::planning::TaskKind Kind>
void append_call_argument(NumericalArgument argument, EvaluationContext<Kind>& context, EvaluatedCallArguments<Kind>& target)
{
    target.numerical_arguments.push_back(evaluate_argument(argument, context));
}

template<typename C, tyr::planning::TaskKind Kind>
auto evaluate_call_arguments(tyr::View<tyr::Index<Rule<CallTag>>, C> rule, EvaluationContext<Kind>& context)
{
    auto result = EvaluatedCallArguments<Kind> {};
    for (const auto& argument : rule.get_call_arguments())
        argument.apply([&](auto typed_argument) { append_call_argument(typed_argument, context, result); });
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
std::optional<ModuleView> resolve_callee(tyr::View<tyr::Index<Rule<CallTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    if (rule.get_data().callee_name.size() != 0)
        return context.find_module(rule.get_callee_name());
    return ModuleView(rule.get_data().callee, context.get_repository());
}

template<typename C, tyr::planning::TaskKind Kind>
RuleExecutionStatus execute_call(tyr::View<tyr::Index<Rule<CallTag>>, C> rule, EvaluationContext<Kind>& context)
{
    if (!has_current_source(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    if (!conditions_are_compatible(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    auto arguments = evaluate_call_arguments(rule, context);
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

template<typename C, tyr::planning::TaskKind Kind>
RuleExecutionStatus execute_immediate_external_rule(tyr::View<tyr::Index<RuleVariant>, C> rule,
                                                    EvaluationContext<Kind>& context,
                                                    const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    return tyr::visit(
        [&](auto concrete_rule)
        {
            using RuleView = std::decay_t<decltype(concrete_rule)>;
            if constexpr (std::same_as<RuleView, tyr::View<tyr::Index<Rule<DoTag>>, C>>)
                return execute_do(concrete_rule, context, successors);
            else if constexpr (std::same_as<RuleView, tyr::View<tyr::Index<Rule<CallTag>>, C>>)
                return execute_call(concrete_rule, context);
            else
                return RuleExecutionStatus::NOT_APPLICABLE;
        },
        rule.get_variant());
}

template<tyr::planning::TaskKind Kind>
RuleExecutionStatus execute_next_immediate_external_rule(EvaluationContext<Kind>& context, const std::vector<tyr::planning::LabeledNode<Kind>>& successors)
{
    const auto module = context.get_module();
    for (const auto& transition : module.get_memory_transitions())
    {
        if (transition.source != context.get_memory_state().get_index())
            continue;

        for (auto rule : tyr::make_view(transition.rules, context.get_repository()))
        {
            const auto status = execute_immediate_external_rule(rule, context, successors);
            if (status != RuleExecutionStatus::NOT_APPLICABLE)
                return status;
        }
    }

    return RuleExecutionStatus::NO_APPLICABLE_RULE;
}

template<typename RuleView, tyr::planning::TaskKind Kind>
bool is_applicable_immediate_external_rule(RuleView rule, EvaluationContext<Kind>& context)
{
    if constexpr (std::same_as<RuleView, tyr::View<tyr::Index<Rule<DoTag>>, Repository>>
                  || std::same_as<RuleView, tyr::View<tyr::Index<Rule<CallTag>>, Repository>>)
        return has_current_source(rule, context) && conditions_are_compatible(rule, context);
    else
        return false;
}

template<tyr::planning::TaskKind Kind>
std::optional<RuleVariantView> find_applicable_immediate_external_rule(EvaluationContext<Kind>& context)
{
    const auto module = context.get_module();
    for (const auto& transition : module.get_memory_transitions())
    {
        if (transition.source != context.get_memory_state().get_index())
            continue;

        for (auto rule : tyr::make_view(transition.rules, context.get_repository()))
        {
            const auto applicable =
                tyr::visit([&](auto concrete_rule) { return is_applicable_immediate_external_rule(concrete_rule, context); }, rule.get_variant());
            if (applicable)
                return rule;
        }
    }

    return std::nullopt;
}

}  // namespace runir::kr::ps::ext

#endif
