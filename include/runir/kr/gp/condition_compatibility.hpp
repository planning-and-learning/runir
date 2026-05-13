#ifndef RUNIR_KR_GP_CONDITION_COMPATIBILITY_HPP_
#define RUNIR_KR_GP_CONDITION_COMPATIBILITY_HPP_

#include "runir/kr/gp/declarations.hpp"

#include <concepts>
#include <tyr/common/types.hpp>

namespace runir::kr::gp
{

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsConditionVariantView = IsEvaluationContext<LanguageTag, EvaluationContext>
                                 && requires(tyr::View<tyr::Index<ConditionVariant>, StorageContext> condition, EvaluationContext& context) {
                                        { condition.is_compatible_with(context) } -> std::same_as<bool>;
                                    };

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteConditionVariantView =
    IsEvaluationContext<LanguageTag, EvaluationContext>
    && requires(tyr::View<tyr::Index<ConcreteConditionVariant<LanguageTag>>, StorageContext> condition, EvaluationContext& context) {
           { condition.is_compatible_with(context) } -> std::same_as<bool>;
       };

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteConditionView =
    IsEvaluationContext<LanguageTag, EvaluationContext>
    && requires(tyr::View<tyr::Index<ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>, StorageContext> condition, EvaluationContext& context) {
           { condition.is_compatible_with(context) } -> std::same_as<bool>;
       };

}  // namespace runir::kr::gp

#endif
