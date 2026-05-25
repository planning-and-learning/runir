#ifndef RUNIR_KR_PS_EXT_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_EXT_COMPATIBILITY_HPP_

#include "runir/kr/ps/ext/condition_view.hpp"
#include "runir/kr/ps/ext/dl/compatibility.hpp"
#include "runir/kr/ps/ext/effect_view.hpp"
#include "runir/kr/ps/ext/rule_variant_view.hpp"
#include "runir/kr/ps/ext/rule_view.hpp"

#include <concepts>
#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>

namespace runir::kr::ps::ext
{

template<typename LanguageTag, typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<ConcreteConditionVariant<LanguageTag>>, C> condition, EvaluationContext& context)
{
    return tyr::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, condition.get_variant());
}

template<typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<ConditionVariant>, C> condition, EvaluationContext& context)
{
    return tyr::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, condition.get_variant());
}

template<typename LanguageTag, typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<ConcreteEffectVariant<LanguageTag>>, C> effect, EvaluationContext& context)
{
    return tyr::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, effect.get_variant());
}

template<typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<EffectVariant>, C> effect, EvaluationContext& context)
{
    return tyr::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, effect.get_variant());
}

template<RuleKind Kind, typename C, typename EvaluationContext>
bool conditions_are_compatible(tyr::View<tyr::Index<Rule<Kind>>, C> rule, EvaluationContext& context)
{
    for (auto condition : rule.get_conditions())
        if (!runir::kr::ps::ext::is_compatible_with(condition, context))
            return false;

    return true;
}

template<RuleKind Kind, typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<Rule<Kind>>, C> rule, EvaluationContext& context)
{
    if (!conditions_are_compatible(rule, context))
        return false;

    if constexpr (std::same_as<Kind, SketchTag>)
    {
        for (auto effect : rule.get_effects())
            if (!runir::kr::ps::ext::is_compatible_with(effect, context))
                return false;
    }

    return true;
}

template<typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<RuleVariant>, C> rule, EvaluationContext& context)
{
    return tyr::visit([&](auto child) { return runir::kr::ps::ext::is_compatible_with(child, context); }, rule.get_variant());
}

}  // namespace runir::kr::ps::ext

#endif
