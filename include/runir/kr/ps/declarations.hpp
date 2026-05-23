#ifndef RUNIR_KR_PS_DECLARATIONS_HPP_
#define RUNIR_KR_PS_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

namespace runir::kr::ps
{

using runir::kr::BaseFamilyTag;
using runir::kr::ExtFamilyTag;
using runir::kr::FamilyTag;

// Feature

template<typename FeatureTag>
struct Feature;

template<typename LanguageTag, typename FeatureTag>
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

struct ConditionVariant
{
};

template<typename LanguageTag>
struct ConcreteConditionVariant;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteCondition;

// Effect

struct EffectVariant
{
};

template<typename LanguageTag>
struct ConcreteEffectVariant;

template<typename LanguageTag, typename FeatureTag, typename ObservationTag>
struct ConcreteEffect;

// Rule

struct Rule
{
};

// Sketch

struct Sketch
{
};

}  // namespace runir::kr::ps

#endif
