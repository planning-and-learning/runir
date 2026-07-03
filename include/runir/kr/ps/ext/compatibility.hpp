#ifndef RUNIR_KR_PS_EXT_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_EXT_COMPATIBILITY_HPP_

#include "runir/kr/ps/ext/condition_view.hpp"
#include "runir/kr/ps/ext/dl/compatibility.hpp"
#include "runir/kr/ps/ext/effect_view.hpp"
#include "runir/kr/ps/ext/evaluation_context.hpp"
#include "runir/kr/ps/ext/evaluation_environment.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"

#include <concepts>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::ext
{

template<typename LanguageTag, typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<ConcreteConditionVariant<LanguageTag>>, C> condition,
                        EvaluationContext<Kind>& context,
                        EvaluationEnvironment<Kind>& environment)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context, environment); }, condition.get_variant());
}

template<typename C, tyr::planning::TaskKind Kind>
bool is_compatible_with(ygg::View<ygg::Index<ConditionVariant>, C> condition, EvaluationContext<Kind>& context, EvaluationEnvironment<Kind>& environment)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context, environment); }, condition.get_variant());
}

template<RuleKind Kind, typename Category, typename C, tyr::planning::TaskKind TaskKind>
bool conditions_are_compatible(ygg::View<ygg::Index<Rule<Kind, Category>>, C> rule,
                               EvaluationContext<TaskKind>& context,
                               EvaluationEnvironment<TaskKind>& environment)
{
    for (auto condition : rule.get_conditions())
        if (!runir::kr::ps::ext::is_compatible_with(condition, context, environment))
            return false;

    return true;
}

template<RuleKind Kind, typename Category, typename C, tyr::planning::TaskKind TaskKind>
bool is_compatible_with(ygg::View<ygg::Index<Rule<Kind, Category>>, C> rule, EvaluationContext<TaskKind>& context, EvaluationEnvironment<TaskKind>& environment)
{
    if (!conditions_are_compatible(rule, context, environment))
        return false;

    if constexpr (requires { rule.get_effects(); })
    {
        for (auto effect : rule.get_effects())
            if (!runir::kr::ps::ext::is_compatible_with(effect, context, environment))
                return false;
    }

    return true;
}

template<typename LanguageTag, typename C, typename EvaluationContext>
bool is_compatible_with(ygg::View<ygg::Index<ConcreteConditionVariant<LanguageTag>>, C> condition, EvaluationContext& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, condition.get_variant());
}

template<typename C, typename EvaluationContext>
bool is_compatible_with(ygg::View<ygg::Index<ConditionVariant>, C> condition, EvaluationContext& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, condition.get_variant());
}

template<typename LanguageTag, typename C, typename EvaluationContext>
bool is_compatible_with(ygg::View<ygg::Index<ConcreteEffectVariant<LanguageTag>>, C> effect, EvaluationContext& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, effect.get_variant());
}

template<typename C, typename EvaluationContext>
bool is_compatible_with(ygg::View<ygg::Index<EffectVariant>, C> effect, EvaluationContext& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, effect.get_variant());
}

template<RuleKind Kind, typename Category, typename C, typename EvaluationContext>
bool conditions_are_compatible(ygg::View<ygg::Index<Rule<Kind, Category>>, C> rule, EvaluationContext& context)
{
    for (auto condition : rule.get_conditions())
        if (!runir::kr::ps::ext::is_compatible_with(condition, context))
            return false;

    return true;
}

template<RuleKind Kind, typename Category, typename C, typename EvaluationContext>
bool is_compatible_with(ygg::View<ygg::Index<Rule<Kind, Category>>, C> rule, EvaluationContext& context)
{
    if (!conditions_are_compatible(rule, context))
        return false;

    if constexpr (requires { rule.get_effects(); })
    {
        for (auto effect : rule.get_effects())
            if (!runir::kr::ps::ext::is_compatible_with(effect, context))
                return false;
    }

    return true;
}

template<typename C, typename EvaluationContext>
bool is_compatible_with(ygg::View<ygg::Index<RuleVariant>, C> rule, EvaluationContext& context)
{
    return ygg::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, rule.get_variant());
}

}  // namespace runir::kr::ps::ext

#endif
