#ifndef RUNIR_KR_GP_DECLARATIONS_HPP_
#define RUNIR_KR_GP_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

#include <concepts>
#include <tyr/common/types.hpp>

namespace runir::kr::gp
{

// Feature

template<typename FeatureTag>
struct Feature;

template<typename LanguageTag, typename FeatureTag>
struct ConcreteFeature;

// EvaluationContext

template<typename LanguageTag>
struct EvaluationContext;

template<typename LanguageTag, typename Context>
concept IsEvaluationContext = requires(const Context& context) {
    context.get_source_state();
    context.get_target_state();
};

// Condition

struct ConditionVariant
{
};

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsConditionVariantView = IsEvaluationContext<LanguageTag, EvaluationContext>
                                 && requires(tyr::View<tyr::Index<ConditionVariant>, StorageContext> condition, EvaluationContext& context) {
                                        { condition.evaluate(context) } -> std::same_as<bool>;
                                    };

template<typename LanguageTag>
struct ConcreteConditionVariant;

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteConditionVariantView =
    IsEvaluationContext<LanguageTag, EvaluationContext>
    && requires(tyr::View<tyr::Index<ConcreteConditionVariant<LanguageTag>>, StorageContext> condition, EvaluationContext& context) {
           { condition.evaluate(context) } -> std::same_as<bool>;
       };

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteCondition;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteConditionView =
    IsEvaluationContext<LanguageTag, EvaluationContext>
    && requires(tyr::View<tyr::Index<ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>>, StorageContext> condition, EvaluationContext& context) {
           { condition.evaluate(context) } -> std::same_as<bool>;
       };

// Effect

struct EffectVariant
{
};

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsEffectVariantView =
    IsEvaluationContext<LanguageTag, EvaluationContext> && requires(tyr::View<tyr::Index<EffectVariant>, StorageContext> effect, EvaluationContext& context) {
        { effect.evaluate(context) } -> std::same_as<bool>;
    };

template<typename LanguageTag>
struct ConcreteEffectVariant;

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteEffectVariantView =
    IsEvaluationContext<LanguageTag, EvaluationContext>
    && requires(tyr::View<tyr::Index<ConcreteEffectVariant<LanguageTag>>, StorageContext> effect, EvaluationContext& context) {
           { effect.evaluate(context) } -> std::same_as<bool>;
       };

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteEffect;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag, typename EvaluationContext, typename StorageContext>
concept IsConcreteEffectView =
    IsEvaluationContext<LanguageTag, EvaluationContext>
    && requires(tyr::View<tyr::Index<ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>>, StorageContext> effect, EvaluationContext& context) {
           { effect.evaluate(context) } -> std::same_as<bool>;
       };

// Rule

struct Rule
{
};

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsRuleView =
    IsEvaluationContext<LanguageTag, EvaluationContext> && requires(tyr::View<tyr::Index<Rule>, StorageContext> rule, EvaluationContext& context) {
        { rule.evaluate(context) } -> std::same_as<bool>;
    };

// Policy

struct Policy
{
};

template<typename LanguageTag, typename EvaluationContext, typename StorageContext>
concept IsPolicyView =
    IsEvaluationContext<LanguageTag, EvaluationContext> && requires(tyr::View<tyr::Index<Policy>, StorageContext> policy, EvaluationContext& context) {
        { policy.evaluate(context) } -> std::same_as<bool>;
    };

}  // namespace runir::kr::gp

#endif
