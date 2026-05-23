#ifndef RUNIR_KR_PS_BASE_DECLARATIONS_HPP_
#define RUNIR_KR_PS_BASE_DECLARATIONS_HPP_

#include "runir/kr/ps/declarations.hpp"

namespace runir::kr::ps::base
{

using FamilyTag = runir::kr::BaseFamilyTag;

template<typename FeatureTag>
using Feature = runir::kr::ps::Feature<FamilyTag, FeatureTag>;

template<typename LanguageTag, typename FeatureTag>
using ConcreteFeature = runir::kr::ps::ConcreteFeature<FamilyTag, LanguageTag, FeatureTag>;

using ConditionVariant = runir::kr::ps::ConditionVariant<FamilyTag>;

template<typename LanguageTag>
using ConcreteConditionVariant = runir::kr::ps::ConcreteConditionVariant<FamilyTag, LanguageTag>;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
using ConcreteCondition = runir::kr::ps::ConcreteCondition<FamilyTag, LanguageTag, FeatureTag, ObservationTag>;

using EffectVariant = runir::kr::ps::EffectVariant<FamilyTag>;

template<typename LanguageTag>
using ConcreteEffectVariant = runir::kr::ps::ConcreteEffectVariant<FamilyTag, LanguageTag>;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
using ConcreteEffect = runir::kr::ps::ConcreteEffect<FamilyTag, LanguageTag, FeatureTag, ObservationTag>;

using Rule = runir::kr::ps::Rule<FamilyTag>;
using Sketch = runir::kr::ps::Sketch<FamilyTag>;

}  // namespace runir::kr::ps::base

#endif
