#ifndef RUNIR_KR_PS_EFFECT_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_EFFECT_COMPATIBILITY_HPP_

#include "runir/kr/ps/declarations.hpp"

#include <concepts>
#include <tyr/common/types.hpp>

namespace runir::kr::ps
{

template<typename Family, typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsEffectVariantView =
    IsEvaluationContext<Family, LanguageTag, EvaluationContext> && requires(tyr::View<tyr::Index<EffectVariant>, StorageContext> effect, EvaluationContext& context) {
        { effect.is_compatible_with(context) } -> std::same_as<bool>;
    };

template<typename Family, typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteEffectVariantView =
    IsEvaluationContext<Family, LanguageTag, EvaluationContext>
    && requires(tyr::View<tyr::Index<ConcreteEffectVariant<LanguageTag>>, StorageContext> effect, EvaluationContext& context) {
           { effect.is_compatible_with(context) } -> std::same_as<bool>;
       };

template<typename Family, typename LanguageTag, typename FeatureTag, typename ObservationTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteEffectView =
    IsEvaluationContext<Family, LanguageTag, EvaluationContext>
    && requires(tyr::View<tyr::Index<ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>>, StorageContext> effect, EvaluationContext& context) {
           { effect.is_compatible_with(context) } -> std::same_as<bool>;
       };

}  // namespace runir::kr::ps

#endif
