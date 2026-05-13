#ifndef RUNIR_KR_GP_EFFECT_COMPATIBILITY_HPP_
#define RUNIR_KR_GP_EFFECT_COMPATIBILITY_HPP_

#include "runir/kr/gp/declarations.hpp"

#include <concepts>
#include <tyr/common/types.hpp>

namespace runir::kr::gp
{

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsEffectVariantView =
    IsEvaluationContext<LanguageTag, EvaluationContext> && requires(tyr::View<tyr::Index<EffectVariant>, StorageContext> effect, EvaluationContext& context) {
        { effect.is_compatible_with(context) } -> std::same_as<bool>;
    };

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteEffectVariantView =
    IsEvaluationContext<LanguageTag, EvaluationContext>
    && requires(tyr::View<tyr::Index<ConcreteEffectVariant<LanguageTag>>, StorageContext> effect, EvaluationContext& context) {
           { effect.is_compatible_with(context) } -> std::same_as<bool>;
       };

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteEffectView =
    IsEvaluationContext<LanguageTag, EvaluationContext>
    && requires(tyr::View<tyr::Index<ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>>, StorageContext> effect, EvaluationContext& context) {
           { effect.is_compatible_with(context) } -> std::same_as<bool>;
       };

}  // namespace runir::kr::gp

#endif
