#ifndef RUNIR_KR_PS_BASE_DECLARATIONS_HPP_
#define RUNIR_KR_PS_BASE_DECLARATIONS_HPP_

#include "runir/kr/ps/declarations.hpp"

namespace runir::kr::ps::base
{

using FamilyTag = runir::kr::BaseFamilyTag;

template<typename FeatureTag>
using Feature = runir::kr::ps::Feature<FeatureTag>;

template<typename LanguageTag, typename FeatureTag>
using ConcreteFeature = runir::kr::ps::ConcreteFeature<LanguageTag, FeatureTag>;

template<typename LanguageTag>
using EvaluationContext = runir::kr::ps::EvaluationContext<FamilyTag, LanguageTag>;

template<typename LanguageTag, typename Context>
concept IsEvaluationContext = runir::kr::ps::IsEvaluationContext<FamilyTag, LanguageTag, Context>;

using ConditionVariant = runir::kr::ps::ConditionVariant;

template<typename LanguageTag>
using ConcreteConditionVariant = runir::kr::ps::ConcreteConditionVariant<LanguageTag>;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
using ConcreteCondition = runir::kr::ps::ConcreteCondition<LanguageTag, FeatureTag, ObservationTag>;

using EffectVariant = runir::kr::ps::EffectVariant;

template<typename LanguageTag>
using ConcreteEffectVariant = runir::kr::ps::ConcreteEffectVariant<LanguageTag>;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
using ConcreteEffect = runir::kr::ps::ConcreteEffect<LanguageTag, FeatureTag, ObservationTag>;

using Rule = runir::kr::ps::Rule;
using Sketch = runir::kr::ps::Sketch;

}  // namespace runir::kr::ps::base

#endif
