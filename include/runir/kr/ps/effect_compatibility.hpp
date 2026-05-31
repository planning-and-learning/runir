#ifndef RUNIR_KR_PS_EFFECT_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_EFFECT_COMPATIBILITY_HPP_

#include "runir/kr/ps/compatibility.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <concepts>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps
{

template<typename Family, typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsEffectVariantView = IsEvaluationContext<Family, LanguageTag, EvaluationContext>
                              && requires(ygg::View<ygg::Index<EffectVariant<Family>>, StorageContext> effect, EvaluationContext& context) {
                                     { runir::kr::ps::is_compatible_with(effect, context) } -> std::same_as<bool>;
                                 };

template<typename Family, typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteEffectVariantView =
    IsEvaluationContext<Family, LanguageTag, EvaluationContext>
    && requires(ygg::View<ygg::Index<ConcreteEffectVariant<Family, LanguageTag>>, StorageContext> effect, EvaluationContext& context) {
           { runir::kr::ps::is_compatible_with(effect, context) } -> std::same_as<bool>;
       };

template<typename Family, typename LanguageTag, typename FeatureTag, typename ObservationTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteEffectView =
    IsEvaluationContext<Family, LanguageTag, EvaluationContext>
    && requires(ygg::View<ygg::Index<ConcreteEffect<Family, LanguageTag, FeatureTag, ObservationTag>>, StorageContext> effect, EvaluationContext& context) {
           { runir::kr::ps::is_compatible_with(effect, context) } -> std::same_as<bool>;
       };

}  // namespace runir::kr::ps

#endif
