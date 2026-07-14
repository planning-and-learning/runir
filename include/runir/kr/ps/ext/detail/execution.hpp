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
    MALFORMED_CALL
};

template<runir::kr::dl::CategoryTag Category, typename C, tyr::planning::TaskKind Kind>
bool has_current_source(ygg::View<ygg::Index<Rule<LoadTag<Category>>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_call_stack().memory_state().get_index();
}

template<typename C, tyr::planning::TaskKind Kind>
bool has_current_source(ygg::View<ygg::Index<Rule<SketchTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_call_stack().memory_state().get_index();
}

template<typename C, tyr::planning::TaskKind Kind>
bool has_current_source(ygg::View<ygg::Index<Rule<DoTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_call_stack().memory_state().get_index();
}

template<typename C, tyr::planning::TaskKind Kind>
bool has_current_source(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    return rule.get_source().get_index() == context.get_call_stack().memory_state().get_index();
}

template<runir::kr::dl::CategoryTag Category, typename C, tyr::planning::TaskKind Kind>
bool load_rule_is_applicable(ygg::View<ygg::Index<Rule<LoadTag<Category>>>, C> rule, EvaluationContext<Kind>& context, EvaluationEnvironment<Kind>& environment)
{
    return has_current_source(rule, context) && conditions_are_compatible(rule, context, environment);
}

template<runir::kr::dl::CategoryTag Category, typename C, tyr::planning::TaskKind Kind, typename Value>
void apply_load_binding(ygg::View<ygg::Index<Rule<LoadTag<Category>>>, C> rule, const Value& value, EvaluationContext<Kind>& context)
{
    if constexpr (std::same_as<Category, runir::kr::dl::ConceptTag>)
        context.get_call_stack().registers().set(rule.get_register().get_identifier(),
                                                 ygg::make_view(value.get_index(), *context.get_state().get_repository()));
    else if constexpr (std::same_as<Category, runir::kr::dl::RoleTag>)
        context.get_call_stack().registers().set(rule.get_register().get_identifier(),
                                                 ygg::make_view(value.first.get_index(), *context.get_state().get_repository()),
                                                 ygg::make_view(value.second.get_index(), *context.get_state().get_repository()));
    else
        static_assert(ygg::dependent_false<Category>::value, "unhandled load rule category");
    context.get_call_stack().set_memory_state(rule.get_target());
}

template<typename C, tyr::planning::TaskKind Kind>
auto& evaluate_do_arguments(ygg::View<ygg::Index<Rule<DoTag>>, C> rule, EvaluationContext<Kind>& context, EvaluationEnvironment<Kind>& environment)
{
    auto action_arguments = rule.get_action_arguments();
    auto& denotations = environment.prepare_do_argument_denotations(action_arguments.size());
    for (auto argument : action_arguments)
        denotations.push_back(ygg::visit([&](auto feature) { return evaluate(feature.get_feature(), context, environment); }, argument.get_variant()));
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
        if (!denotations[index].get().test(ygg::uint_t(objects[index].get_index())))
            return false;

    return true;
}

template<typename C, tyr::planning::TaskKind Kind>
bool do_rule_is_applicable(ygg::View<ygg::Index<Rule<DoTag>>, C> rule, EvaluationContext<Kind>& context, EvaluationEnvironment<Kind>& environment)
{
    return has_current_source(rule, context) && conditions_are_compatible(rule, context, environment);
}

template<typename ConceptDenotations, typename C, tyr::planning::TaskKind Kind>
bool do_successor_matches(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                          EvaluationContext<Kind>& context,
                          EvaluationEnvironment<Kind>& environment,
                          const ConceptDenotations& denotations,
                          tyr::formalism::planning::GroundActionView action,
                          const tyr::planning::StateView<Kind>& target_state)
{
    if (!action_matches_do_arguments(rule, action, denotations))
        return false;

    auto dl_context = environment.make_dl_transition_context(context, target_state);
    return is_compatible_with(rule, dl_context);
}

// Whether a Do rule selects the concrete transition (context state -> target_state) taken by
// `action`: memory match + conditions on the source + action name/argument match + effects
// across the transition.
template<typename C, tyr::planning::TaskKind Kind>
bool do_rule_matches(ygg::View<ygg::Index<Rule<DoTag>>, C> rule,
                     EvaluationContext<Kind>& context,
                     EvaluationEnvironment<Kind>& environment,
                     tyr::formalism::planning::GroundActionView action,
                     const tyr::planning::StateView<Kind>& target_state)
{
    if (!do_rule_is_applicable(rule, context, environment))
        return false;

    const auto& denotations = evaluate_do_arguments(rule, context, environment);
    return do_successor_matches(rule, context, environment, denotations, action, target_state);
}

template<typename C, tyr::planning::TaskKind Kind>
void apply_do_successor(ygg::View<ygg::Index<Rule<DoTag>>, C> rule, const tyr::planning::LabeledNode<Kind>& successor, EvaluationContext<Kind>& context)
{
    context.get_state() = successor.node.get_state();
    context.get_call_stack().set_memory_state(rule.get_target());
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
        context.get_call_stack().set_memory_state(rule.get_target());
        return RuleExecutionStatus::APPLIED;
    }

    const auto successor = select_sketch_successor(rule, context, environment, successors);
    if (!successor)
        return RuleExecutionStatus::NOT_APPLICABLE;

    context.get_state() = successor->node.get_state();
    context.get_call_stack().set_memory_state(rule.get_target());
    return RuleExecutionStatus::APPLIED;
}

template<typename FeatureTag, typename C, tyr::planning::TaskKind Kind>
void append_call_argument(ygg::View<ygg::Index<runir::kr::ps::Feature<runir::kr::ExtFamilyTag, FeatureTag>>, C> argument,
                          EvaluationContext<Kind>& context,
                          EvaluationEnvironment<Kind>& environment,
                          EvaluationArguments& target)
{
    const auto denotation = evaluate_feature_denotation(argument, context, environment);
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        target.get<runir::kr::dl::ConceptTag>().push_back(denotation);
    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
        target.get<runir::kr::dl::RoleTag>().push_back(denotation);
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
        target.get<runir::kr::dl::BooleanTag>().push_back(denotation);
    else
        target.get<runir::kr::dl::NumericalTag>().push_back(denotation);
}

template<typename C, tyr::planning::TaskKind Kind>
auto& evaluate_call_arguments(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, EvaluationContext<Kind>& context, EvaluationEnvironment<Kind>& environment)
{
    auto& result = context.get_call_stack().prepare_call_arguments();
    rule.for_each_call_argument([&](auto argument) { append_call_argument(argument, context, environment, result); });
    return result;
}

inline bool call_arguments_match_signature(ModuleView callee, const EvaluationArguments& arguments)
{
    return arguments.get<runir::kr::dl::ConceptTag>().size() == callee.template get_arguments<runir::kr::dl::ConceptTag>().size()
           && arguments.get<runir::kr::dl::RoleTag>().size() == callee.template get_arguments<runir::kr::dl::RoleTag>().size()
           && arguments.get<runir::kr::dl::BooleanTag>().size() == callee.template get_arguments<runir::kr::dl::BooleanTag>().size()
           && arguments.get<runir::kr::dl::NumericalTag>().size() == callee.template get_arguments<runir::kr::dl::NumericalTag>().size();
}

template<typename C, tyr::planning::TaskKind Kind>
std::optional<ModuleView> resolve_callee(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, const EvaluationContext<Kind>& context)
{
    return context.get_program().find_module(rule.get_callee().get_index());
}

template<typename C, tyr::planning::TaskKind Kind>
RuleExecutionStatus execute_call(ygg::View<ygg::Index<Rule<CallTag>>, C> rule, EvaluationContext<Kind>& context, EvaluationEnvironment<Kind>& environment)
{
    if (!has_current_source(rule, context))
        return RuleExecutionStatus::NOT_APPLICABLE;

    if (!conditions_are_compatible(rule, context, environment))
        return RuleExecutionStatus::NOT_APPLICABLE;

    auto& arguments = evaluate_call_arguments(rule, context, environment);
    const auto callee = resolve_callee(rule, context);
    if (!callee || !call_arguments_match_signature(*callee, arguments))
        return RuleExecutionStatus::MALFORMED_CALL;

    context.get_call_stack().enter_module(*callee, rule.get_target(), std::move(arguments));
    return RuleExecutionStatus::APPLIED;
}

}  // namespace runir::kr::ps::ext::detail

#endif
