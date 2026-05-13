#ifndef RUNIR_KR_GP_DECLARATIONS_HPP_
#define RUNIR_KR_GP_DECLARATIONS_HPP_

#include "runir/kr/declarations.hpp"

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

// Policy

struct Policy
{
};

}  // namespace runir::kr::gp

#endif
