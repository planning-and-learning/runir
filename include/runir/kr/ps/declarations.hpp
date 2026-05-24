#ifndef RUNIR_KR_PS_DECLARATIONS_HPP_
#define RUNIR_KR_PS_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

namespace runir::kr::ps
{

using runir::kr::FamilyTag;

// Feature

template<FamilyTag Family, typename FeatureTag>
struct Feature;

template<FamilyTag Family, typename LanguageTag, typename FeatureTag>
struct ConcreteFeature;

// EvaluationContext

template<FamilyTag Family, typename LanguageTag>
struct EvaluationContext;

template<typename Family, typename LanguageTag, typename Context>
concept IsEvaluationContext = FamilyTag<Family> && requires(const Context& context) {
    context.get_source_state();
    context.get_target_state();
};

// Condition

template<FamilyTag Family>
struct ConditionVariant
{
};

template<FamilyTag Family, typename LanguageTag>
struct ConcreteConditionVariant;

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteCondition;

// Effect

template<FamilyTag Family>
struct EffectVariant
{
};

template<FamilyTag Family, typename LanguageTag>
struct ConcreteEffectVariant;

template<FamilyTag Family, typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteEffect;

}  // namespace runir::kr::ps

#endif
