#ifndef RUNIR_KR_PS_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_COMPATIBILITY_HPP_

#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/effect_view.hpp"

#include <tyr/common/types.hpp>
#include <tyr/common/variant.hpp>

namespace runir::kr::ps
{

template<runir::kr::FamilyTag Family, typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<ConditionVariant<Family>>, C> condition, EvaluationContext& context)
{
    return tyr::visit([&](auto child) { return runir::kr::ps::is_compatible_with(child, context); }, condition.get_variant());
}

template<runir::kr::FamilyTag Family, typename LanguageTag, typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<ConcreteConditionVariant<Family, LanguageTag>>, C> condition, EvaluationContext& context)
    requires runir::kr::ps::IsEvaluationContext<typename EvaluationContext::Family, LanguageTag, EvaluationContext>
{
    return tyr::visit([&](auto child) { return runir::kr::ps::is_compatible_with(child, context); }, condition.get_variant());
}

template<runir::kr::FamilyTag Family, typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<EffectVariant<Family>>, C> effect, EvaluationContext& context)
{
    return tyr::visit([&](auto child) { return runir::kr::ps::is_compatible_with(child, context); }, effect.get_variant());
}

template<runir::kr::FamilyTag Family, typename LanguageTag, typename C, typename EvaluationContext>
bool is_compatible_with(tyr::View<tyr::Index<ConcreteEffectVariant<Family, LanguageTag>>, C> effect, EvaluationContext& context)
    requires runir::kr::ps::IsEvaluationContext<typename EvaluationContext::Family, LanguageTag, EvaluationContext>
{
    return tyr::visit([&](auto child) { return runir::kr::ps::is_compatible_with(child, context); }, effect.get_variant());
}

}  // namespace runir::kr::ps

#endif
