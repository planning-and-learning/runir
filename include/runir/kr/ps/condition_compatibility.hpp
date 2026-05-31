#ifndef RUNIR_KR_PS_CONDITION_COMPATIBILITY_HPP_
#define RUNIR_KR_PS_CONDITION_COMPATIBILITY_HPP_

#include "runir/kr/ps/compatibility.hpp"
#include "runir/kr/ps/declarations.hpp"

#include <concepts>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps
{

template<typename Family, typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsConditionVariantView = IsEvaluationContext<Family, LanguageTag, EvaluationContext>
                                 && requires(ygg::View<ygg::Index<ConditionVariant<Family>>, StorageContext> condition, EvaluationContext& context) {
                                        { runir::kr::ps::is_compatible_with(condition, context) } -> std::same_as<bool>;
                                    };

template<typename Family, typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteConditionVariantView =
    IsEvaluationContext<Family, LanguageTag, EvaluationContext>
    && requires(ygg::View<ygg::Index<ConcreteConditionVariant<Family, LanguageTag>>, StorageContext> condition, EvaluationContext& context) {
           { runir::kr::ps::is_compatible_with(condition, context) } -> std::same_as<bool>;
       };

template<typename Family, typename LanguageTag, typename FeatureTag, typename ObservationTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteConditionView =
    IsEvaluationContext<Family, LanguageTag, EvaluationContext>
    && requires(ygg::View<ygg::Index<ConcreteCondition<Family, LanguageTag, FeatureTag, ObservationTag>>, StorageContext> condition,
                EvaluationContext& context) {
           { runir::kr::ps::is_compatible_with(condition, context) } -> std::same_as<bool>;
       };

}  // namespace runir::kr::ps

#endif
